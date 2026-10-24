VERSION = 0.77.2

CC	= cc

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
DATADIR = $(PREFIX)/share/astro

LIBS = -lform -lmenu -lpanel -lncurses $$( [ "$$(uname -s)" = Linux ] && printf '%s' -ltinfo) -lm

CPPFLAGS =	-Isrc -Iswisseph -DASTRO_DATA_DIR='"$(DATADIR)"' -DVERSION=\"$(VERSION)\"
CFLAGS	= 	-Wall -Wextra -Wpedantic -O3

SRC		=	src/anim.c src/astro.c src/chronos.c src/conf.c \
			src/draw.c src/indat.c src/init.c src/io.c \
			src/search.c src/ui.c src/zr.c src/diary.c

SWE_SRC = 	swisseph/swedate.c swisseph/swehouse.c swisseph/swejpl.c \
			swisseph/swemmoon.c swisseph/swemplan.c swisseph/sweph.c \
			swisseph/swephlib.c swisseph/swecl.c swisseph/swehel.c
			
SWE_OBJ	=	${SWE_SRC:.c=.o}
SWE_D	=	${SWE_SRC:.c=.d}
SWE_A	=	swisseph/libswe.a

all: astro

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@
-include $(SWE_D)
	
$(SWE_A): $(SWE_OBJ)
	ar rcs $@ $(SWE_OBJ)
	
astro: ${SWE_A}
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRC) $(SWE_A) $(LIBS) -o $@
	
debug: ${SWE_A}
	$(CC) $(CPPFLAGS) $(CFLAGS) -g -O0 $(SRC) $(SWE_A) $(LIBS) -o astro
	
clean:
	rm -f $(SWE_OBJ) $(SWE_D) $(SWE_A) astro

install: all
	mkdir -p "$(DESTDIR)$(BINDIR)"
	cp -f "astro" "$(DESTDIR)$(BINDIR)"
	chmod 755 "$(DESTDIR)$(BINDIR)/astro"
	mkdir -p "$(DESTDIR)$(DATADIR)"
	cp -R "swisseph/ephe" "$(DESTDIR)$(DATADIR)"
	cp city-db "$(DESTDIR)$(DATADIR)"

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/astro
	rm -rf $(DESTDIR)$(DATADIR)

.PHONY: all astro debug clean install uninstall
