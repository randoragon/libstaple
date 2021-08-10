# Modify these 3 numbers to bump librnd version
VERSION_MAJOR = 1
VERSION_MINOR = 0
VERSION_PATCH = 0
VERSION_STR   = $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

# Last version modification date (YYYY-MM-DD)
DATE = 2021-07-26

# Compilation / Linking
CC = cc
LINKER = cc
VFLAGS =  -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR)
VFLAGS += -DVERSION_PATCH=$(VERSION_PATCH) -DVERSION_STR="$(VERSION_STR)"
CFLAGS = -fpic $(VFLAGS) -std=c89 -Wall -Wextra -pedantic -Werror -Werror=vla
LDFLAGS = -shared
CTESTFLAGS = -std=c89 -Wall -Wextra -pedantic -Werror=vla -g -Og
LDTESTFLAGS = -L. -Wl,-Bstatic -lrnd -Wl,-Bdynamic

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
MANPAGES3 := $(subst $(MANDIR)/,,$(wildcard $(MANDIR)/*.3))
MANPAGES7 := $(subst $(MANDIR)/,,$(wildcard $(MANDIR)/*.7))

# Main library file
TARGET = librnd

# Output paths
DESTDIR =
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

.PHONY: directories static shared all main clean debug profile install uninstall test test-stack


##################################################################################################
##################################################################################################


# Compiles the library (both static and shared)
all: directories static shared

# Creates directories for all build processes
directories:
	mkdir -p $(SRCDIR) $(OBJDIR) $(TESTDIR) $(TESTDIR)/$(SRCDIR) $(TESTDIR)/$(OBJDIR) $(TESTDIR)/bin

# Builds a shared library file
shared: $(OBJS)
	$(LINKER) $(OBJS) $(LDFLAGS) -o $(TARGET).so

# Builds a static library file
static: $(OBJS)
	$(AR) -rcs $(TARGET).a $(OBJS)

# Compiles a library source file into an object file
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c $(CFLAGS) $^ -o $@

# Compiles a test source file into an object file
$(TESTDIR)/$(OBJDIR)/%.o: $(TESTDIR)/$(SRCDIR)/%.c
	$(CC) -c $(CTESTFLAGS) $^ -o $@

# Removes all object and output files
clean:
	$(RM) $(OBJS)
	$(RM) $(TESTDIR)/$(OBJDIR)/*
	$(RM) $(TARGET).so $(TARGET).a

# Rebuilds the library with debug symbols and RND_DEBUG defined
debug: CFLAGS += -g -Og -DRND_DEBUG
debug: clean all

# Builds and installs the library
install: CFLAGS += -O3
install: clean all
	@mkdir -p -- $(DESTDIR)$(PREFIX)/lib $(DESTDIR)$(PREFIX)/include $(DESTDIR)$(MANPREFIX)/man3 $(DESTDIR)$(MANPREFIX)/man7
	install -m644 -- $(TARGET).so $(DESTDIR)$(PREFIX)/lib
	install -m644 -- $(TARGET).a $(DESTDIR)$(PREFIX)/lib
	for f in $(INCLUDES); do install -m644 -- "$(SRCDIR)/$$f" $(DESTDIR)$(PREFIX)/include/"$$f"; done
	for f in $(MANPAGES3); do sed -e "s/VERSION/$(VERSION_STR)/g" -e "s/DATE/$(DATE)/g" < "$(MANDIR)/$$f" > $(DESTDIR)$(MANPREFIX)/man3/"$$f"; chmod 644 -- $(DESTDIR)$(MANPREFIX)/man3/"$$f"; done
	for f in $(MANPAGES7); do sed -e "s/VERSION/$(VERSION_STR)/g" -e "s/DATE/$(DATE)/g" < "$(MANDIR)/$$f" > $(DESTDIR)$(MANPREFIX)/man7/"$$f"; chmod 644 -- $(DESTDIR)$(MANPREFIX)/man7/"$$f"; done
	@echo Successfully installed.

# Removes installed library files from the system (opposite of "install")
uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/$(TARGET).so
	$(RM) -- $(DESTDIR)$(PREFIX)/lib/$(TARGET).a
	for f in $(INCLUDES); do $(RM) -- $(DESTDIR)$(PREFIX)/include/"$$f"; done
	for f in $(MANPAGES3); do $(RM) -- $(DESTDIR)$(MANPREFIX)/man3/"$$f"; done
	for f in $(MANPAGES7); do $(RM) -- $(DESTDIR)$(MANPREFIX)/man7/"$$f"; done

# Runs all testing units
#     To check if overflow protection is working,
#     set SIZE_MAX to 65535 to reduce memory footprint.
test: CFLAGS += -DRND_QUIET -DSIZE_MAX=65535
test: test-stack test-queue

test-stack: debug test/obj/test_struct.o test/obj/stack.o
	$(LINKER) "test/obj/test_struct.o" "test/obj/stack.o" $(LDTESTFLAGS) -o $(TESTDIR)/bin/stack
	@tput setaf 4
	@printf "\n##########"
	@tput setaf 3
	@printf "[ STACK ]"
	@tput setaf 4
	@printf "##########\n\n"
	@tput setaf 7
	valgrind ./test/bin/stack
	@tput setaf 4
	@printf "\n###########"
	@tput setaf 3
	@printf "[ END ]"
	@tput setaf 4
	@printf "###########\n\n"
	@tput setaf 7

test-queue: debug test/obj/test_struct.o test/obj/queue.o
	$(LINKER) "test/obj/test_struct.o" "test/obj/queue.o" $(LDTESTFLAGS) -o $(TESTDIR)/bin/queue
	@tput setaf 4
	@printf "\n##########"
	@tput setaf 3
	@printf "[ QUEUE ]"
	@tput setaf 4
	@printf "##########\n\n"
	@tput setaf 7
	valgrind ./test/bin/queue
	@tput setaf 4
	@printf "\n###########"
	@tput setaf 3
	@printf "[ END ]"
	@tput setaf 4
	@printf "###########\n\n"
	@tput setaf 7
