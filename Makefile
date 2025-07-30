CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11
LDFLAGS = -lncurses
TARGET = seashell
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

.PHONY: all clean
