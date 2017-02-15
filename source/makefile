#makefile
CC=gcc -Wall
LDFLAGS = -pthread
all: server client

server : server_main.o share.o
				$(CC) -o server server_main.o share.o

client : client_main.o share.o
				$(CC) $(LDFLAGS) -o client client_main.o share.o

server_main.o: server_main.c
				$(CC) -c server_main.c

client_main.o : client_main.c
				$(CC) $(LDFLAGS) -c client_main.c

share.o : share.c
				$(CC) -c share.c

clean:
				rm -f *.o core.* server client
