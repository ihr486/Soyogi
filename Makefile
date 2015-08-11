CSRCS := $(wildcard *.c)

COBJS := $(CSRCS:%.c=%.o)

OBJS := $(COBJS)

BIN := decoder

CC := gcc
CFLAGS := -Wall -Wextra -std=c99 -O2 -DDEBUG
LDFLAGS := -lm

.PHONY: all check clean

all: $(BIN)

check:
	$(CC) $(CFLAGS) -fsyntax-only $(CSRCS)

clean:
	-@rm -vf $(OBJS) $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
