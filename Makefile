CC=gcc
CFLAGS=-lcrypt -Wall -Wextra -std=gnu99 -O2

SRCS = $(wildcard src/*.c)

all: kchat

kchat: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $@

clean:
	rm -f kchat
