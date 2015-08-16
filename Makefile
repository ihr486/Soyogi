CSRCS := $(wildcard *.c)
HDRS := $(wildcard *.h)

COBJS := $(CSRCS:%.c=%.o)

OBJS := $(COBJS)

BIN := decoder

CC := gcc
CFLAGS := -Wall -Wextra -std=c99 -O2 -DDEBUG $(shell pkg-config --cflags libpulse libpulse-simple)
LDFLAGS := -lm $(shell pkg-config --libs libpulse libpulse-simple)

.PHONY: all check clean

all: $(BIN)

check:
	$(CC) $(CFLAGS) -fsyntax-only $(CSRCS)

clean:
	-@rm -vf $(OBJS) $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c -o $@ $<
