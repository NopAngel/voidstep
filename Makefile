# Makefile for voidstep
# Copyright (C) 2026 NopAngel
#
# This file is part of the voidstep project.
# Licensed under the GNU General Public License v3 or later.

# Compiler and flags
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g
LDFLAGS = 

# Target binary name
TARGET  = voidstep

Q 	    = @

# Source files
SRCS    = src/main.c
OBJS    = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "  LD  $@ "
	$(Q)$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)


%.o: %.c
	$(Q)echo "  CC  $< "
	$(Q)$(CC) $(CFLAGS) -c $< -o $@


clean:
	$(Q)rm -f $(OBJS) $(TARGET)
	$(Q)echo "  CLEAN  done"


install: $(TARGET)
	$(Q)echo "  INSTALL  $(TARGET) to /usr/local/bin/"
	$(Q)sudo install -m 755 $(TARGET) /usr/local/bin/



.PHONY: all clean install
