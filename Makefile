CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -Iinclude
LDFLAGS = -lncurses
TARGET = seashell
DATE = 8.23.25
VERSION = 1.3.0
SRC = src/main.c \
      src/loop/loop.c \
      src/loop/parser.c \
      src/loop/executor.c \
      src/variable_handling/aliases.c \
      src/variable_handling/env.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
sign:
	gpg --output seashell.sig --detach-sig seashell

package:
	tar -cvf SeaShell-$(VERSION)-$(DATE).tar.xz $(TARGET) $(TARGET).sig
	rm $(TARGET) $(TARGET).sig
.PHONY: all clean
