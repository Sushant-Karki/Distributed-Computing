all: client.o server.o
	gcc client.o -g -o client
	gcc server.o -g -o server
client.o: client.c header.h
	gcc	-g -c -o client.o	client.c
server.o: server.c header.h
	gcc	-g -c -o server.o	server.c
