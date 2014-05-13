CPPFLAGS = -Wall -Wextra -lupyun -lcurl
LDFLAGS = -lupyun -lcurl

all: upc

upc: pull.o util.o main.o push.o stage.o
	$(CC) -g -o $@ $+ $(LDFLAGS) 

clean: 
	rm -f pull.o util.o main.o push.o stage.o upc
