CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11
TARGET = seashell
SRC = src/main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean
