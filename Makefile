CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11

TARGET = processflow
SRC = src/main.c src/processflow.c

all: $(TARGET)

$(TARGET): $(SRC) src/processflow.h
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean