LDFLAGS=-pthread

all: kchat

kchat: command.c server.c

clean:
	rm -f kchat
