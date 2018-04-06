src = $(wildcard *.c)
obj = $(patsubst %.c, %.o, $(src))

all: server client

server: server.o wrap.o
	gcc server.o wrap.o -lpthread -Wall -g -o server
client: client.o wrap.o
	gcc client.o wrap.o -Wall -g -o client

%.o:%.c
	gcc -c -std=c99 $< -Wall

.PHONY: clean all
clean:
	-rm -rf server client $(obj)
