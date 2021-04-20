CC=gcc
CFLAGS=-lcrypt -Wall -Wextra

SRCS = $(wildcard src/*.c)

all: kchat

kchat: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $@

clean:
	rm -f kchat
