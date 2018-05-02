all: server client

server: server.o server_select.o server_poll.o wrap.o
	gcc server.o server_select.o server_poll.o wrap.o -I ./ -Wall -lpthread -o server

client: client.o wrap.o
	gcc client.o wrap.o -I ./ -Wall -lpthread -o client

%.o:%.c
	gcc -c $< -Wall -g	

.PHONY:clean all
clean:
	rm -rf server client *.o
