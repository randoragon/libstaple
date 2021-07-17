# Modify these 3 numbers to bump librnd version
VERSION_MAJOR = 1
VERSION_MINOR = 0
VERSION_PATCH = 0
VERSION_STR   = $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

# Last major modification date (YYYY-MM-DD)
DATE = 2021-07-17

# Compilation / Linking
CC = cc
LINKER = cc
VFLAGS =  -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR)
VFLAGS += -DVERSION_PATCH=$(VERSION_PATCH) -DVERSION_STR="$(VERSION_STR)"
CFLAGS = -fpic $(VFLAGS) -std=c89 -Wall -Wextra -pedantic -Werror -Werror=vla
LDFLAGS = -shared
CTESTFLAGS = -std=c89 -Wall -Wextra -pedantic -Werror=vla -g -Og
LDTESTFLAGS = -L . -lrnd

# Directories
SRCDIR = src
OBJDIR = obj
MANDIR = man
TESTDIR = test

# Source and object files (including SRCDIR)
SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%, $(OBJDIR)/%, $(SRCS:.c=.o))

# Header and man page files (only filenames)
INCLUDES := $(subst $(SRCDIR)/,,$(wildcard $(SRCDIR)/rnd*.h))
MANPAGES := $(subst $(MANDIR)/,,$(wildcard $(MANDIR)/*.3))

# Main library file
TARGET = librnd.so

# Output paths
DESTDIR =
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

.PHONY: directories all main clean debug profile install uninstall test test-stack

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
	$(RM) $(OBJS)
	$(RM) $(TESTDIR)/$(OBJDIR)/*

debug: CFLAGS += -g -Og -DRND_DEBUG
debug: clean all

profile: CFLAGS += -pg
profile: LDFLAGS += -pg
profile: clean all

install: CFLAGS += -O3
install: clean all
	@mkdir -p -- $(DESTDIR)$(PREFIX)/lib $(DESTDIR)$(PREFIX)/include $(DESTDIR)$(MANPREFIX)/man3
	install -m644 -- $(TARGET) $(DESTDIR)$(PREFIX)/lib
	for f in $(INCLUDES); do install -m644 -- "$(SRCDIR)/$$f" $(DESTDIR)$(PREFIX)/include/"$$f"; done
	for f in $(MANPAGES); do sed -e "s/VERSION/$(VERSION_STR)/g" -e "s/DATE/$(DATE)/g"< "$(MANDIR)/$$f" > $(DESTDIR)$(MANPREFIX)/man3/"$$f"; chmod 644 -- $(DESTDIR)$(MANPREFIX)/man3/"$$f"; done

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/$(TARGET)
	for f in $(INCLUDES); do $(RM) -- $(DESTDIR)$(PREFIX)/include/"$$f"; done
	for f in $(MANPAGES); do $(RM) -- $(DESTDIR)$(MANPREFIX)/man3/"$$f"; done

# To check if overflow protection is working,
# set SIZE_MAX to 65535 to reduce memory footprint.
test: CFLAGS += -DRND_QUIET -DSIZE_MAX=65535
test: test-stack

test-stack: debug test/obj/test_struct.o test/obj/stack.o
	$(LINKER) $(LDTESTFLAGS)  "test/obj/test_struct.o" "test/obj/stack.o" -o $(TESTDIR)/bin/stack
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
