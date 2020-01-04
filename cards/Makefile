CFLAGS=-O3 -fno-strict-aliasing -pipe -Wall -Wextra -fpic -DPIC -D_REENTRANT
CC=gcc

SRCS=$(wildcard *.c)
SRCS!=ls *.c
OBJS=${SRCS:.c=.o}
LOBJS=${SRCS:.c=.lo}

.SUFFIXES: .o .c
.PHONY: clean rebuild

.c.o:
	@echo Building $<...
	@libtool --quiet --mode=compile $(CC) $(CFLAGS) -c $< 
	@echo $@ built.

libccards.la: $(OBJS)
	@echo Building library...
	@libtool --quiet --mode=link $(CC) -o $@ $(CFLAGS) $(LOBJS) -rpath /usr/local/lib 
	@echo $@ built.

test: cards.c
	gcc -o $@ $(CFLAGS) $(SRCS) -DTEST_MAIN

clean:
	@rm -f test
	@rm -f *.lo
	@rm -f *.la
	@rm -f *.o
	@rm -rf .libs

rebuild: clean libccards.la
