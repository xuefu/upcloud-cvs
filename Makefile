CPPFLAGS = -Wall -Wextra -lupyun -lcurl
LDFLAGS = -lupyun -lcurl

all: upc

upc: pull.o util.o main.o push.o stage.o
	$(CC) -o $@ $+ $(LDFLAGS) 

install:
	sudo cp upc /usr/bin/

uninstall:
	sudo rm /usr/bin/upc

clean: 
	rm -f pull.o util.o main.o push.o stage.o upc
