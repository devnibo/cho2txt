PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

all:
	$(CC) -O -Werror -o cho2txt cho2txt.c
clean:
	rm cho2txt
install: all
	mkdir -p "$(PREFIX)/bin"
	cp -f snd "$(PREFIX)/bin"
	chmod 755 "$(PREFIX)/bin/cho2txt"
	mkdir -p "$(MANPREFIX)/man1"
	cp -f snd.1 "$(MANPREFIX)/man1/cho2txt.1"
	chmod 644 "$(MANPREFIX)/man1/cho2txt.1"
uninstall:
	rm "$(PREFIX)/bin/cho2txt"
	rm "$(MANPREFIX)/man1/cho2txt.1"
