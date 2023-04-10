CC = gcc
CFLAGS = -Wall -O2
TARGETS = pc8toppc pc8top86

all: $(TARGETS)

pc8toppc: pc8toppc.c
	$(CC) $(CFLAGS) pc8toppc.c -o pc8toppc

pc8top86: pc8top86.c
	$(CC) $(CFLAGS) pc8top86.c -o pc8top86

clean:
	rm -f $(TARGETS)
