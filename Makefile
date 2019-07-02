LDFLAGS=-pthread

all: server

server: command.c clients.c

clean:
	rm -f server
