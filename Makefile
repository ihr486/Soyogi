CSRCS := $(wildcard *.c)
HDRS := $(wildcard *.h)

COBJS := $(CSRCS:%.c=%.o)

OBJS := $(COBJS)

BIN := soyogi

CC := gcc
CFLAGS := -Wall -Wextra -std=c99 -O3

.PHONY: all check clean install

all: $(BIN)

check:
	$(CC) $(CFLAGS) -fsyntax-only $(CSRCS)

clean:
	-@rm -vf $(OBJS) $(BIN)

install: $(BIN)
	install --mode=755 --target-directory=/usr/local/bin $<

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(HDRS) Makefile
	$(CC) $(CFLAGS) -c -o $@ $<
