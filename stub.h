#ifndef STUB_H
#define STUB_H

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


int initialize_server_connection(char *IP, char *port);
int initialize_client_connection(char *IP, char *port, int num_clients, int mode);
int ready_to_shutdown();
int send_client();
int reader_stuff();
int writter_stuff();
void control_exit();
void priority_readers();
void priority_writters();

#endif
