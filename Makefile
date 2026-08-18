CC = cc
CFLAGS = -Wall -Wextra -pedantic -std=c11

processflow: src/main.c
	$(CC) $(CFLAGS) src/main.c -o processflow

clean:
	rm -f processflow