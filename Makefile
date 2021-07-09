CC = cc
LINKER = cc
CFLAGS = -fpic -std=c89 -Wall -Wextra -pedantic -Werror -Werror=vla
LDFLAGS = -shared

SRCDIR = src
OBJDIR = obj
TESTDIR = test
SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%, $(OBJDIR)/%, $(SRCS:.c=.o))
TARGET = librnd.so
DESTDIR =
PREFIX = /usr/local

.PHONY: directories all main clean debug profile test test-stack

all: directories main

directories:
	mkdir -p $(SRCDIR) $(OBJDIR) $(TESTDIR) $(TESTDIR)/$(SRCDIR) $(TESTDIR)/$(OBJDIR) $(TESTDIR)/bin

main: $(OBJS)
	$(LINKER) $(OBJS) $(LDFLAGS) -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c $(CFLAGS) $^ -o $@

$(TESTDIR)/$(OBJDIR)/%.o: $(TESTDIR)/$(SRCDIR)/%.c
	$(CC) -c $(CFLAGS) $^ -o $@

clean:
	rm -f $(OBJS)

debug: CFLAGS += -g -Og
debug: clean all

profile: CFLAGS += -pg
profile: LDFLAGS += -pg
profile: clean all

test: test-stack

test-stack: main test/obj/stack.o
	$(LINKER) -L . -lrnd "test/obj/stack.o" -o $(TESTDIR)/bin/stack
	LD_LIBRARY_PATH=. ./test/bin/stack
