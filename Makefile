CC = cc
LINKER = cc
CFLAGS = -fpic -std=c89 -Wall -Wextra -pedantic -Werror -Werror=vla
LDFLAGS = -shared
CTESTFLAGS = -std=c89 -Wall -Wextra -pedantic -Werror=vla -g -Og
LDTESTFLAGS = -L . -lrnd

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
	$(CC) -c $(CTESTFLAGS) $^ -o $@

clean:
	rm -f $(OBJS)
	rm -f $(TESTDIR)/$(OBJDIR)/*

debug: CFLAGS += -g -Og -DRND_DEBUG
debug: clean all

profile: CFLAGS += -pg
profile: LDFLAGS += -pg
profile: clean all

# To check if overflow protection is working,
# set SIZE_MAX to 65535 to reduce memory footprint.
test: CFLAGS += -DRND_QUIET -DSIZE_MAX=65535
test: test-stack

test-stack: debug test/obj/stack.o
	$(LINKER) $(LDTESTFLAGS) "test/obj/stack.o" -o $(TESTDIR)/bin/stack
	@tput setaf 4
	@printf "\n##########"
	@tput setaf 3
	@printf "[ BEGIN ]"
	@tput setaf 4
	@printf "##########\n\n"
	@tput setaf 7
	LD_LIBRARY_PATH=. valgrind ./test/bin/stack
	@tput setaf 4
	@printf "\n###########"
	@tput setaf 3
	@printf "[ END ]"
	@tput setaf 4
	@printf "###########\n\n"
	@tput setaf 7
