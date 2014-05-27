CPPFLAGS = -Wall -Wextra -lupyun -lcurl
LDFLAGS = -lupyun -lcurl

all: upc

upc: pull.o util.o main.o push.o stage.o color.o md5_file.o
	$(CC) -o $@ $+ $(LDFLAGS) 

install:
	@(sudo cp upc /usr/bin/)
	@echo "installing upc ..."
	@echo "Finished!"

uninstall:
	@(sudo rm /usr/bin/upc)
	@echo "uninstalling upc ..."
	@echo "Finished!"

clean: 
	@(rm -f pull.o util.o main.o push.o stage.o color.o md5_file.o upc)
