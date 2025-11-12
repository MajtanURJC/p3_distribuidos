#ifndef STUB_H
#define STUB_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

struct sockets {
    int client_sock;   
    int server_sock;  
};

extern struct sockets sockets;

extern pthread_mutex_t rw_mutex;       
extern pthread_mutex_t mutex_readers;  
extern pthread_cond_t cond;            
extern int readers_count;              
extern int writers_waiting;            
extern int priority_readers;          


void init_sync_primitives();
void destroy_sync_primitives();


void set_priority_readers();   
void set_priority_writers();   


void reader_enter();   
void reader_exit();    
void writer_enter();   
void writer_exit();    


void send_message(int sock, const char *msg);
void receive_message(int sock, char *msg, size_t size);


int initialize_server_connection(char *IP, char *port);
int initialize_client_connection(char *IP, char *port);

#endif
