LDFLAGS=-pthread

all: server

server: command.c client.c

clean:
	rm -f server
