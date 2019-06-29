LDFLAGS=-pthread

all: server

server: client.c

clean:
	rm -f server
