LDFLAGS=-pthread

all: kchat

kchat: $(wildcard *.c)

clean:
	rm -f kchat
