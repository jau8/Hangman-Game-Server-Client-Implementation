CC=g++

OS := $(shell uname -s)

# Extra LDFLAGS if Solaris
ifeq ($(OS), SunOS)
	LDFLAGS=-lsocket -lnsl
    endif

all: client server 

# -lpthread is used for pthread
client: client.cpp
	$(CC) client.cpp -lpthread -o client

server: server.cpp
	$(CC) server.cpp -lpthread -o server

clean:
	    rm -f client server *.o

