CC := cc
SRC := src/main.c src/installer.c src/parser.c src/resolver.c src/uninstaller.c src/util.c src/commands/help.c src/commands/make.c src/commands/remove.c src/commands/show.c
OBJS := $(SRC:.c=.o)
LDLIBS := -ljansson

PREFIX ?= /usr/local
BINDIR := $(PREFIX)/bin

all: virt-pack

virt-pack: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

install: all
	mkdir -p $(BINDIR)
	install virt-pack $(BINDIR)
	install scripts/update_db.py $(BINDIR)

clean:
	rm -f virt-pack src/*.o

