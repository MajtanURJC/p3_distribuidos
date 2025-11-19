#include "stub.h"

int mode;

sem_t sem_threads;
pthread_mutex_t slot_mutex = PTHREAD_MUTEX_INITIALIZER;

int unjoined_threads = 0;
pthread_t temp_threads[MAX_THREADS];
int thread_free[MAX_THREADS];

pthread_mutex_t writer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reader_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mix_mutex = PTHREAD_MUTEX_INITIALIZER;

int num_writer = 0;
int writers_waiting = 0;
int writer_active = 0;
int readers_waiting = 0;

pthread_cond_t ok_read  = PTHREAD_COND_INITIALIZER;
pthread_cond_t ok_write = PTHREAD_COND_INITIALIZER;

int priority_mode;   // 0 = prioridad a lectores, 1 = prioridad a escritores
int initial_val = 0;



int search_free(){
    for(int i = 0; i < MAX_THREADS; i++) {
        pthread_mutex_lock(&slot_mutex);
        if(thread_free[i] == 1) {
            thread_free[i] = 0; //Evitar condiciones de carrera de que se sobrescriba
            pthread_mutex_unlock(&slot_mutex);
            return i;
        }
        pthread_mutex_unlock(&slot_mutex);
    }
    return -1;
}


void *receive_loop_client(void  *arg) {
    setbuf(stdout, NULL);
    
    struct client_info *info = (struct client_info*)arg;
    struct sockets sockets;
    int bytes_sended;
    struct response response;
    int bytes_received;
    char text_oper[10];

    sockets.client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sockets.client_sock < 0) {
        perror("Error on socket");
        return NULL;
    }

    struct sockaddr_in server_addr;
    unsigned short host_port = (unsigned short) info->port;  //PORT

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(info->ip);  //IP
    server_addr.sin_port = htons(host_port);

    if (connect(sockets.client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error on connect");
        close(sockets.client_sock);
        return NULL;
    }

    
    bytes_sended = send(sockets.client_sock, &mode, sizeof(mode), 0);
    if (bytes_sended < 0) {
        perror("Error on send");
        close(sockets.client_sock);
        return NULL;
    }

    bytes_received = recv(sockets.client_sock, &response, sizeof(response), 0);
    if (bytes_received < 0) {
        perror("Error on recv");
        return NULL;
    }

    if ( response.action == 0 ) {
        strcpy(text_oper,"Escritor");
    } else {
        strcpy(text_oper,"Lector");
    }

    printf("Cliente #%d] %s, contador=%d, tiempo=%ld ns.\n",
       info->id, text_oper, response.counter, response.latency_time);

    
    free(info);
    close(sockets.client_sock);
    return NULL;
}


void *receive_loop_server(void *arg) {
    setbuf(stdout, NULL);

    struct arg_thread *info = (struct arg_thread *)arg;

    int sock = info->socket;
    int pos  = info->pos;
    int bytes_received;
    int error;

    

    struct request req;
    bytes_received = recv(sock, &req, sizeof(req), 0);
    if (bytes_received <= 0) { 
        perror("Error on recv");
        return NULL;
    }

    struct response response;
    response.action = req.action;


    if (req.action == 0) {
        error = writer_stuff(req.id,response,sock);
    } else {
        error = reader_stuff(req.id,response,sock);
    }
    
    pthread_mutex_lock(&slot_mutex);
    thread_free[pos] = 1;
    pthread_mutex_unlock(&slot_mutex);


    free(info);
    close(sock);
    sem_post(&sem_threads);
    return NULL;
}


int initialize_server_connection(char *port) {

    FILE *read_val = fopen("server_output.txt", "r");
    if (read_val != NULL) {
        char buffer[6];  
        size_t n = fread(buffer, 1, sizeof(buffer) - 1, read_val);
        buffer[n] = '\0'; 

        char *endptr;
        initial_val = strtol(buffer, &endptr, 10); 

        if (endptr == buffer) {
            printf("No se encontró un número válido en el archivo.\n");
            fclose(read_val);
            return -1;
        } 

        fclose(read_val);
    }
    for(int i=0; i<MAX_THREADS; i++){
        thread_free[i] = 1;
    } 


    unjoined_threads = 0;

    sem_init(&sem_threads, 0, 600);

    char *endptr;
    long valor = strtol(port, &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid Port: %s\n", port);
        return -1;
    }

    struct sockaddr_in server_addr;
    unsigned short host_port = (unsigned short)valor;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error on socket");
        return -1;
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("error(setsockopt(SO_REUSEADDR) failed: ");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(host_port);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error on bind");
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 4096) < 0) {
        perror("Error on listen");
        close(sockfd);
        return -1;
    }

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    struct sockets sockets;

    while (1) {
        
        if(unjoined_threads != -1) {
            sem_wait(&sem_threads);  

            sockets.server_sockets[unjoined_threads] = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
            if (sockets.server_sockets[unjoined_threads] < 0) {
                perror("Error on accept");
                close(sockfd);
                return -1;
            }

            struct arg_thread *args = malloc(sizeof(struct arg_thread));
            if (args == NULL) {
                perror("malloc failed");
                close(sockets.server_sockets[unjoined_threads]);
                sem_post(&sem_threads);  
                continue;                
            }
            args->pos = unjoined_threads;
            args->socket = sockets.server_sockets[unjoined_threads];

            if (pthread_create(&temp_threads[unjoined_threads], NULL, receive_loop_server, 
                        args) != 0) {
                perror("Error on pthread");
                for (int i = 0; i < unjoined_threads; i++) {
                    close(sockets.server_sockets[i]);
                }
                close(sockfd);
                return -1;
            }
            
            pthread_mutex_lock(&slot_mutex);
            thread_free[unjoined_threads] = 0;
            pthread_mutex_unlock(&slot_mutex);
        }

        unjoined_threads = search_free();
    }

    return 0;
}


int initialize_client_connection(char *IP, char *port, int arg_clients, int mode_func) {
    mode = mode_func;
    int num_clients = 0;
    char *endptr;
    long valor = strtol(port, &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid Port: %s\n", port);
        return -1;
    }

    pthread_t client_threads[arg_clients];
    
    while(num_clients < arg_clients) {

        struct client_info *client_info = malloc(sizeof(struct client_info));
        if (client_info == NULL) {
        perror("malloc failed");
        continue;
        }
    
        strcpy(client_info->ip,IP);
        client_info->port = valor; 
        client_info->id = num_clients+1;

        if (pthread_create(&client_threads[num_clients], NULL, receive_loop_client, 
                        client_info) != 0) {
            perror("Error on pthread_create");
            return -1;
        }

        num_clients++;
    }

    for(int i=0; i < arg_clients;i++) {
        pthread_join(client_threads[i],NULL);
    }

    return 0;
}

int writer_stuff(int id, struct response response, int sock) {
    struct timespec t_start, t_enter;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    pthread_mutex_lock(&mix_mutex);
    writers_waiting++;

    while (priority_mode == 0 && readers_waiting > 0)
        pthread_cond_wait(&ok_write, &mix_mutex);

    clock_gettime(CLOCK_MONOTONIC, &t_enter);

    writers_waiting--;
    writer_active = 1;

    num_writer++;

    FILE *fp = fopen("server_output.txt", "w");
    if (fp != NULL) {
        fprintf(fp, "%d\n", num_writer);
        fclose(fp);
    }

    int ms = 75 + rand() % 76;
    usleep(ms * 1000);

    int num = num_writer + initial_val;
    printf("[WRITER #%d] modifica contador con valor %d\n", id, num);

    writer_active = 0;

    if (priority_mode == 0)
        pthread_cond_broadcast(&ok_read);
    else
        pthread_cond_signal(&ok_write);

    response.latency_time = (t_enter.tv_sec * 1000000000L + t_enter.tv_nsec) -
                            (t_start.tv_sec * 1000000000L + t_start.tv_nsec);
    response.counter = num;

    pthread_mutex_unlock(&mix_mutex);

    int sended = send(sock, &response, sizeof(response), 0);
    if (sended < 0) {
        perror("Error on writers send");
        return -1;
    }

    return 0;
}

int reader_stuff(int id, struct response response, int sock) {
    pthread_mutex_lock(&reader_mutex);
    readers_waiting++;
    pthread_mutex_unlock(&reader_mutex);

    struct timespec t_start, t_enter;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    pthread_mutex_lock(&mix_mutex);
    while (writer_active || (priority_mode == 1 && writers_waiting > 0))
        pthread_cond_wait(&ok_read, &mix_mutex);

    clock_gettime(CLOCK_MONOTONIC, &t_enter);
    pthread_mutex_unlock(&mix_mutex);

    pthread_mutex_lock(&reader_mutex);
    readers_waiting--;
    pthread_mutex_unlock(&reader_mutex);

    int ms = 75 + rand() % 76;
    usleep(ms * 1000);

    int num = num_writer + initial_val;
    printf("[READER #%d] lee contador con valor %d\n", id, num);

    pthread_mutex_lock(&mix_mutex);
    if (readers_waiting == 0)
        pthread_cond_signal(&ok_write);

    response.latency_time = (t_enter.tv_sec * 1000000000L + t_enter.tv_nsec) -
                            (t_start.tv_sec * 1000000000L + t_start.tv_nsec);
    response.counter = num;

    int sended = send(sock, &response, sizeof(response), 0);
    if (sended < 0) {
        perror("Error on readers send");
        pthread_mutex_unlock(&mix_mutex);
        return -1;
    }

    pthread_mutex_unlock(&mix_mutex);
    return 0;
}


void set_priority(int priority) {
    priority_mode = priority;
}


