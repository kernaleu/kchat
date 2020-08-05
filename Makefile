CC=clang
CFLAGS=-fsanitize=address

SRCS = $(wildcard src/*.c)

all: kchat

kchat: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $@

clean:
	rm -f kchat
