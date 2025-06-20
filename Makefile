CC=gcc
CFLAGS=-Wall -Iinclude

SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.o)

virt-pack: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	rm -f virt-pack src/*.o
