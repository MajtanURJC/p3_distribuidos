#include "stub.h"

receiver_thread[1000];
int mode;
int counter;
sem_t sem_threads;

void *receive_loop_client(char net_info[]) {
    setbuf(stdout, NULL);
    
    struct sockets sockets;
    struct response response;
    int bytes_sended;
    int bytes_received;
    char text_oper[10];

    sockets.client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sockets.client_sock < 0) {
        perror("Error on socket");
        return -1;
    }

    struct sockaddr_in server_addr;
    unsigned short host_port = (unsigned short) net_info[1];  //PORT

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(net_info[0]);  //IP
    server_addr.sin_port = htons(host_port);

    if (connect(sockets.client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error on connect");
        close(sockets.client_sock);
        return -1;
    }

    
    bytes_sended = send(sockets.client_sock, mode, sizeof(mode), 0);
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
        strcpy(text_oper,"Lector");
    } else {
        strcpy(text_oper,"Escritor");
    }

    printf ( "Cliente #%d] %s, contador=%d, tiempo=%ld ns.\n",
    net_info[2],text_oper, response.latency_time );
    
    close(sockets.client_sock);
    return NULL;
}


void *receive_loop_server(void *arg) {
    setbuf(stdout, NULL);

    int sock = *(int *)arg;
    struct response response;
    int type;
    int bytes_received;
    int bytes_sended;

    
    bytes_received = recv(sock, type, sizeof(type), 0);
    if (bytes_received < 0) {
        perror("Error on recv");
        return NULL;
    }

    if(type == 0) {
        response.latency_time = writter_stuff();
    } else {
        response.latency_time = reader_stuff();
    }

    response.action = type;
    response.counter = counter;

    bytes_sended = send(sock, &response, len(response), 0);
    if (bytes_sended < 0) {
        perror("Error on send");
        close(sock);
        return NULL;
    }

    close(sock);
    sem_post(&sem_threads);
    return NULL;
}


int initialize_server_connection(char *IP, char *port) {

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
    server_addr.sin_addr.s_addr = inet_addr(IP);
    server_addr.sin_port = htons(host_port);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error on bind");
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 600) < 0) {
        perror("Error on listen");
        close(sockfd);
        return -1;
    }

    while (1) {

        sem_wait(&sem_threads);  

        struct sockaddr_in client_addr;
        struct sockets sockets;
        socklen_t client_len = sizeof(client_addr);

        // Revisar lo del sem_val, modo incorrecto, hacer mutex ----------------------------------------------
        int sem_val;

        sem_getvalue(&sem_threads,&sem_val);

        sockets.server_sockets[sem_val] = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (sockets.server_sockets[sem_val] < 0) {
            perror("Error on accept");
            close(sockfd);
            return -1;
        }

        if (pthread_create(&receiver_thread[sem_val], NULL, receive_loop_server, 
                       &sockets.server_sockets[sem_val]) != 0) {
            perror("Error on pthread");
            for (int i = 0; i < sem_val; i++) {
                close(sockets.server_sockets[i]);
            }
            close(sockfd);
            return -1;
        }

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


    char net_info[2];
    net_info[0] = IP;
    net_info[1] = port;
    net_info[2] = num_clients+1;
    
    while(num_clients < arg_clients) {
        if (pthread_create(&receiver_thread[num_clients], NULL, receive_loop_client, 
                        &net_info) != 0) {
            perror("Error on pthread_create");
            return -1;
        }

        num_clients++;
    }
    return 0;
}
