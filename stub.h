#ifndef STUB_H
#define STUB_H

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>
#include <stdatomic.h>

#define MAX_THREADS 600

enum operations {
    WRITE = 0,
    READ
};

struct request {
    enum operations action;
    unsigned int id;
};

struct response {
    enum operations action;
    unsigned int counter;
    long latency_time;
};

struct sockets {
    int client_sock;
    int server_sockets[600];
};

struct arg_thread {
    int socket;
    int pos;
};

struct client_info {
    char ip[64];
    int port;
    int id;
};

int initialize_server_connection(char *port);
int initialize_client_connection(char *IP, char *port, int num_clients, int mode);

int reader_stuff(int id, struct response resp, int sock);
int writer_stuff(int id, struct response resp, int sock);

void set_priority(int priority);

#endif
