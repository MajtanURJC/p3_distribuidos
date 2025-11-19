# Compilador
CC = gcc

# Opciones de compilación
CFLAGS = -Wall -g -I.

# Archivos fuente
COMMON = stub.c
SRC_client = client.c $(COMMON)
SRC_server = server.c $(COMMON)

# Archivos objeto
OBJ_client = $(SRC_client:.c=.o)
OBJ_server = $(SRC_server:.c=.o)


# Regla por defecto
all: client server

# Enlazado
client: $(OBJ_client)
	$(CC) $(CFLAGS) -o $@ $^

server: $(OBJ_server)
	$(CC) $(CFLAGS) -o $@ $^


# Compilación de cada archivo .c a .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpieza
clean:
	rm -f $(OBJ_client) $(OBJ_server) client server
