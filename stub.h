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
#include <pthread.h>

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

int initialize_server_connection(char *IP, char *port);
int initialize_client_connection(char *IP, char *port);


#endif #STUB_H