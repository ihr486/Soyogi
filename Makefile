CSRCS := $(wildcard *.c)
HDRS := $(wildcard *.h)

COBJS := $(CSRCS:%.c=%.o)

OBJS := $(COBJS)

BIN := decoder

CC := gcc
CFLAGS := -Wall -Wextra -std=c99 -O3
LDFLAGS := -lm

.PHONY: all check clean

all: $(BIN)

check:
	$(CC) $(CFLAGS) -fsyntax-only $(CSRCS)

clean:
	-@rm -vf $(OBJS) $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c $(HDRS) Makefile
	$(CC) $(CFLAGS) -c -o $@ $<
