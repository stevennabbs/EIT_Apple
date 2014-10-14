CFLAGS =-std=c99 -D_POSIX_C_SOURCE=200809L -W -Wall -Wextra -O3
LIBS   =

DESTDIR=
PREFIX =/usr/local

INSTALL= install -p
INSTALL_EXEC= $(INSTALL) -m 0755
INSTALL_DATA= $(INSTALL) -m 0644

SRC=src/*.c
HDR=

tagger: $(SRC) $(HDR)
	@echo "CC: tagger.c --> tagger"
	@$(CC) -DNDEBUG $(CFLAGS) -o tagger $(SRC) $(LIBS)

debug: $(SRC) $(HDR)
	@echo "CC: tagger.c --> tagger"
	@$(CC) -g $(CFLAGS) -o tagger $(SRC) $(LIBS)

install: tagger
	@echo "CP: tagger   --> $(DESTDIR)$(PREFIX)/bin"
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1
	@$(INSTALL_EXEC) tagger       $(DESTDIR)$(PREFIX)/bin
	@$(INSTALL_DATA) doc/tagger.1 $(DESTDIR)$(PREFIX)/share/man/man1

clean:
	@echo "RM: tagger"
	@rm -f tagger

.PHONY: clean install
