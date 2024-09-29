CC=gcc
CFLAGS=-lcrypt
LDFLAGS=-Wall -Wextra -std=gnu99 -O2

SRCS = $(wildcard src/*.c)

all: kchat

kchat: $(SRCS)
	$(CC) $(LDFLAGS) $(SRCS) -o $@ $(CFLAGS)

clean:
	rm -f kchat
