#include "stub.h"
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct sockets sockets;
pthread_mutex_t rw_mutex;          
pthread_mutex_t mutex_readers;     
pthread_cond_t cond;               
int readers_count = 0;
int writers_waiting = 0;
int priority_readers = 1;          


int max(int a, int b) { return (a > b) ? a : b; }

void init_sync_primitives() {
    pthread_mutex_init(&rw_mutex, NULL);
    pthread_mutex_init(&mutex_readers, NULL);
    pthread_cond_init(&cond, NULL);
}

void destroy_sync_primitives() {
    pthread_mutex_destroy(&rw_mutex);
    pthread_mutex_destroy(&mutex_readers);
    pthread_cond_destroy(&cond);
}


void set_priority_readers() { 
    priority_readers = 1; 
}

void set_priority_writers() { 
    priority_readers = 0; 
}


void reader_enter() {
    pthread_mutex_lock(&mutex_readers);
    if (!priority_readers) {
        while (writers_waiting > 0) {
            pthread_cond_wait(&cond, &mutex_readers);
        }
    }
    readers_count++;
    if (readers_count == 1) pthread_mutex_lock(&rw_mutex);
    pthread_mutex_unlock(&mutex_readers);
}

void reader_exit() {
    pthread_mutex_lock(&mutex_readers);
    readers_count--;
    if (readers_count == 0) pthread_mutex_unlock(&rw_mutex);
    if (!priority_readers) pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex_readers);
}

void writer_enter() {
    pthread_mutex_lock(&mutex_readers);
    writers_waiting++;
    pthread_mutex_unlock(&mutex_readers);

    pthread_mutex_lock(&rw_mutex);
}

void writer_exit() {
    pthread_mutex_lock(&mutex_readers);
    writers_waiting--;
    pthread_mutex_unlock(&mutex_readers);

    pthread_mutex_unlock(&rw_mutex);
    pthread_cond_broadcast(&cond);
}

// Enviar y recibir mensajes simples
void send_message(int sock, const char *msg) {
    send(sock, msg, strlen(msg) + 1, 0);
}

void receive_message(int sock, char *msg, size_t size) {
    recv(sock, msg, size, 0);
}

// Inicialización de conexión
int initialize_server_connection(char *IP, char *port) {
    int sockfd;
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return -1;

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP);
    server_addr.sin_port = htons(atoi(port));

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 10) < 0) {
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int initialize_client_connection(char *IP, char *port) {
    int sockfd;
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return -1;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP);
    server_addr.sin_port = htons(atoi(port));

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        return -1;
    }

    return sockfd;
}
