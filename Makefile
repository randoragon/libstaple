CC = cc
LINKER = cc
CFLAGS = -fpic -std=c99 -Wall -Wextra -pedantic -Werror -Werror=vla
LDFLAGS = -shared

# All SRCDIR subdirectories that contain source files
DIRS = .

SRCDIR = src
OBJDIR = obj
SRCDIRS := $(foreach dir, $(DIRS), $(addprefix $(SRCDIR)/, $(dir)))
OBJDIRS := $(foreach dir, $(DIRS), $(addprefix $(OBJDIR)/, $(dir)))
SRCS := $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))
OBJS := $(patsubst $(SRCDIR)/%, $(OBJDIR)/%, $(SRCS:.c=.o))
TARGET = librnd.so
DESTDIR =
PREFIX = /usr/local

.PHONY: directories all main clean debug profile install uninstall

all: directories main

directories:
	mkdir -p $(SRCDIRS) $(OBJDIRS)

main: $(OBJS)
	$(LINKER) $(OBJS) $(LDFLAGS) -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c $(CFLAGS) $^ -o $@

clean:
	rm -f $(OBJS)

debug: CFLAGS += -g -Og
debug: clean all

profile: CFLAGS += -pg
profile: LDFLAGS += -pg
profile: clean all

install: CFLAGS += -O3
install: LDFLAGS += -O3
install: clean all
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -- $(TARGET) $(DESTDIR)$(PREFIX)/bin
	@chmod -- 755 $(DESTDIR)$(PREFIX)/$(TARGET)

uninstall:
	rm -f -- $(DESTDIR)$(PREFIX)/bin/$(TARGET)
