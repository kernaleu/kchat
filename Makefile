CC=gcc
CFLAGS=-lcrypt
LDFLAGS=-Wall -Wextra -std=gnu99 -O2

SRCS = $(wildcard src/*.c)

all: kchat

kchat: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $@

clean:
	rm -f kchat
