#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "stub.h"

int main(int argc, char *argv[]) {
    char *port = NULL;
    char *priority = NULL;

    int c, option_index = 0;

    static struct option long_options[] = {
        {"port",     required_argument, 0, 'p'},
        {"priority", required_argument, 0, 'r'},
        {0, 0, 0, 0}
    };

    while ((c = getopt_long(argc, argv, "p:r:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'p':
                port = optarg;
                break;

            case 'r':
                priority = optarg;
                break;

            case '?':
                fprintf(stderr,"Incorrect arguments\n");
                exit(1);
        }
    }

    // Validar argumentos
    if (port == NULL || priority == NULL) {
        fprintf(stderr,"Not enough arguments.\n");
        exit(1);
    }

    // Interpretar prioridad
    if (strcmp(priority, "writer") == 0) {
        set_priority(1);
    } else if (strcmp(priority, "reader") == 0) {
        set_priority(0);
    } else {
        fprintf(stderr,"Unvalid priority\n");
        exit(1);
    }


    int result = initialize_server_connection(port);
    if (result != 0) {
        fprintf(stderr, "Error iniciating the server\n");
        exit(1);
    }

    return 0;
}
