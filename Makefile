CC ?= gcc
CFLAGS = -Wall -O2
PREFIX ?= /usr
TARGETS = pc8toppc pcmtop86 wav2pc8

all: $(TARGETS)

pc8toppc: pc8toppc.c
	$(CC) $(CFLAGS) pc8toppc.c -o pc8toppc

pcmtop86: pcmtop86.c
	$(CC) $(CFLAGS) pcmtop86.c -o pcmtop86

wav2pc8: wav2pc8.c
	$(CC) $(CFLAGS) wav2pc8.c -o wav2pc8

install:
	install -m 0755 -d $(DESTDIR)$(PREFIX)/bin
	install -m 0755 $(TARGETS) $(DESTDIR)$(PREFIX)/bin

uninstall:
	-rm $(DESTDIR)$(PREFIX)/bin/pc8toppc
	-rm $(DESTDIR)$(PREFIX)/bin/pcmtop86
	-rm $(DESTDIR)$(PREFIX)/bin/wav2pc8

clean:
	rm -f $(TARGETS)
