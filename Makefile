# Modify these 3 numbers to bump libstaple version
VERSION_MAJOR := 2
VERSION_MINOR := 1
VERSION_PATCH := 0
VERSION_STR   := $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

# Last version modification date (YYYY-MM-DD)
DATE := 2022-05-03

# Main library file
TARGET := libstaple

# Compilation / Linking
CC          := cc
LINKER      := cc
VFLAGS      := -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR)
VFLAGS      += -DVERSION_PATCH=$(VERSION_PATCH) -DVERSION_STR="$(VERSION_STR)"
STDC        := c99  # Change to newer standards, e.g. c99 to enable more functions
CFLAGS      := -fpic $(VFLAGS) -std=$(STDC) -Wall -Wextra -pedantic -Werror -Werror=vla
CFLAGS      += $(if $(filter 1,$(DEBUG)),-DSTAPLE_DEBUG -g -Og,-O3)
CFLAGS      += $(if $(filter 1,$(QUIET)),-DSTAPLE_QUIET)
CFLAGS      += $(if $(filter 1,$(ABORT)),-DSTAPLE_ABORT)
LDFLAGS     := -shared
CTESTFLAGS  := -std=c99 -Wall -Wextra -pedantic -Werror=vla -g -Og
LDTESTFLAGS := -L. -l:./$(TARGET).so -lcriterion

# List of all library module names
MODULES := stack queue

# Directories
SRCDIR  := src
OBJDIR  := obj
MANDIR  := man
GENDIR  := gen
TEMPLATEDIR := $(GENDIR)/templates
TESTDIR := test
SRCSUBDIRS := . internal utils $(MODULES)

# Source and object files (including SRCDIR)
SRCDIRS := $(foreach dir, $(SRCSUBDIRS), $(addprefix $(SRCDIR)/, $(dir)))
OBJDIRS := $(foreach dir, $(SRCSUBDIRS), $(addprefix $(OBJDIR)/, $(dir)))
SRCS    := $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))
OBJS    := $(patsubst $(SRCDIR)/%, $(OBJDIR)/%, $(SRCS:.c=.o))

# Header and man page files (relative to MANDIR)
INCLUDES  := $(notdir $(wildcard $(SRCDIR)/staple*.h $(SRCDIR)/sp_*.h))
MANPAGES3 := $(patsubst $(MANDIR)/%,%,$(shell find "$(MANDIR)" -type f -name '*.3'))
MANPAGES7 := $(patsubst $(MANDIR)/%,%,$(shell find "$(MANDIR)" -type f -name '*.7'))

# Template files
TEMPLATES := $(patsubst $(TEMPLATEDIR)/%,%,$(shell find "$(TEMPLATEDIR)" -type f))

# Output paths
DESTDIR   :=
PREFIX    := /usr/local
MANPREFIX := $(PREFIX)/share/man

.PHONY: directories static shared all generate clean install uninstall test
.SECONDARY: # Disable removal of intermediate files

##################################################################################################
##################################################################################################


# Compiles the library (both static and shared)
all: directories static shared

# Creates directories for all build processes
directories:
	@mkdir -p -- $(SRCDIRS) $(OBJDIRS) $(TESTDIR)
	@mkdir -p -- $(TESTDIR)/$(SRCDIR) $(TESTDIR)/$(OBJDIR) $(TESTDIR)/bin
	@mkdir -p -- $(dir $(TEMPLATES))

# Builds a shared library file
shared: $(OBJS)
	@printf 'LD\tCreating dynamic library file... '
	@$(LINKER) $(OBJS) $(LDFLAGS) -o $(TARGET).so
	@echo 'done.'

# Builds a static library file
static: $(OBJS)
	@printf 'AR\tCreating static library file... '
	@$(AR) -rcs $(TARGET).a $(OBJS)
	@echo 'done.'

# Compiles a library source file into an object file
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@printf 'CC\t%s\n' $^
	@$(CC) -c $(CFLAGS) $^ -o $@

# Compiles a test source file into an object file
$(TESTDIR)/$(OBJDIR)/%.o: $(TESTDIR)/$(SRCDIR)/%.c
	@printf 'CC\t%s\n' $^
	@$(CC) -c $(CTESTFLAGS) $^ -o $@

# Generates the source code from templates
generate: directories
	@$(GENDIR)/generate $(TEMPLATES)

# Removes all object and output files
clean:
	@printf 'RM\tCleaning object files and binaries... '
	@$(RM) -- $(OBJS)
	@$(RM) -- $(TARGET).so $(TARGET).a
	@echo 'done.'

# Builds and installs the library
install: all
	@printf 'MKDIR\tCreating directories... '
	@mkdir -p -- $(DESTDIR)$(PREFIX)/lib $(DESTDIR)$(PREFIX)/include $(DESTDIR)$(MANPREFIX)/man3 $(DESTDIR)$(MANPREFIX)/man7
	@echo 'done.'
	@
	@printf 'INSTALL\tInstalling library files... '
	@install -m644 -- $(TARGET).so $(DESTDIR)$(PREFIX)/lib
	@install -m644 -- $(TARGET).a $(DESTDIR)$(PREFIX)/lib
	@echo 'done.'
	@
	@printf 'INSTALL\tInstalling header files... '
	@for f in $(INCLUDES); do install -m644 -- "$(SRCDIR)/$$f" $(DESTDIR)$(PREFIX)/include/"$$f"; done
	@echo 'done.'
	@
	@printf 'INSTALL\tInstalling man pages... '
	@for f in $(MANPAGES3); do sed -e "s/VERSION/$(VERSION_STR)/g" -e "s/DATE/$(DATE)/g" < "$(MANDIR)/$$f" | gzip -7 - > $(DESTDIR)$(MANPREFIX)/man3/"$${f##*/}.gz"; chmod 644 -- $(DESTDIR)$(MANPREFIX)/man3/"$${f##*/}.gz"; done
	@for f in $(MANPAGES7); do sed -e "s/VERSION/$(VERSION_STR)/g" -e "s/DATE/$(DATE)/g" < "$(MANDIR)/$$f" | gzip -7 - > $(DESTDIR)$(MANPREFIX)/man7/"$${f##*/}.gz"; chmod 644 -- $(DESTDIR)$(MANPREFIX)/man7/"$${f##*/}.gz"; done
	@echo 'done.'

# Removes installed library files from the system (opposite of "install")
uninstall:
	@printf 'RM\tUninstalling library files... '
	@$(RM) -- $(DESTDIR)$(PREFIX)/lib/$(TARGET).so
	@$(RM) -- $(DESTDIR)$(PREFIX)/lib/$(TARGET).a
	@echo 'done.'
	@
	@printf 'RM\tUninstalling library files... '
	@for f in $(INCLUDES); do $(RM) -- $(DESTDIR)$(PREFIX)/include/"$$f"; done
	@echo 'done.'
	@
	@printf 'RM\tUninstalling man pages... '
	@for f in $(MANPAGES3); do $(RM) -- $(DESTDIR)$(MANPREFIX)/man3/"$${f##*/}.gz"; done
	@for f in $(MANPAGES7); do $(RM) -- $(DESTDIR)$(MANPREFIX)/man7/"$${f##*/}.gz"; done
	@echo 'done.'

# Runs all testing units
#     To check if overflow protection is working,
#     set SP_SIZE_MAX to 65535 to reduce memory footprint.
test: $(addprefix test_,$(MODULES))

test_clean:
	@printf 'RM\tCleaning testing object files and binaries... '
	@$(RM) -- $(TESTDIR)/$(OBJDIR)/*
	@$(RM) -- $(TESTDIR)/bin/*
	@echo 'done.'

test_%: CFLAGS += -DSTAPLE_DEBUG -DSTAPLE_QUIET -DSP_SIZE_MAX=65535
test_%: all test/obj/test_struct.o test/obj/%.o
	@printf 'LD\tLinking test programs... '
	@$(LINKER) test/obj/test_struct.o test/obj/$*.o $(LDTESTFLAGS) -o $(TESTDIR)/bin/$*
	@echo 'done.'
	@
	@tput setaf 4 ; printf "\n##########" ; tput setaf 3
	@printf "[ $* ]"
	@tput setaf 4 ; printf "##########\n\n" ; tput setaf 7
	valgrind ./test/bin/$*
	@tput setaf 4 ; printf "\n###########" ; tput setaf 3
	@printf "[ END ]"
	@tput setaf 4 ; printf "###########\n\n" ; tput setaf 7
