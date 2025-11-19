#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "stub.h"

int main(int argc, char *argv[]) {
    char *ip = NULL;
    char *port_str = NULL;
    char *mode_str = NULL;
    int threads = 0;
    int mode = 0;

    int option_index = 0;
    int c;

    static struct option long_options[] = {
        {"ip", required_argument, 0, 'i'},
        {"port", required_argument, 0, 'p'},
        {"mode", required_argument, 0, 'm'},
        {"threads", required_argument, 0, 't'},
        {0, 0, 0, 0}
    };

    while ((c = getopt_long(argc, argv, "i:p:m:t:", long_options, &option_index)) != -1) {
        switch(c) {
            case 'i':
                ip = strdup(optarg);
                break;
            case 'p':
                port_str = strdup(optarg);
                break;
            case 'm':
                mode_str = strdup(optarg);
                break;
            case 't':
                threads = atoi(optarg);
                break;
            case '?':
            default:
                fprintf(stderr, "Uso: %s --ip <IP> --port <PORT> --mode writer/reader --threads <NUM>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (ip == NULL || port_str == NULL || mode_str == NULL || threads <= 0) {
        fprintf(stderr, "Not enought arguments\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(mode_str, "writer") == 0) {
        mode = 0;
    } else if (strcmp(mode_str, "reader") == 0) {
        mode = 1;
    } else {
        fprintf(stderr, "Modo inválido: %s (usar 'writer' o 'reader')\n", mode_str);
        exit(EXIT_FAILURE);
    }

    if (initialize_client_connection(ip, port_str, threads, mode) != 0) {
        fprintf(stderr, "Error al conectar con el servidor.\n");
        exit(EXIT_FAILURE);
    }

    free(ip);
    free(port_str);
    free(mode_str);

    return 0;
}
