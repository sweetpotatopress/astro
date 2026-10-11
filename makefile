CC	= cc
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
DATADIR = $(PREFIX)/share/astro

CFLAGS	:= -Wall -Wextra -Wpedantic -O3
CPPFLAGS = -Isrc -Iswisseph -DASTRO_DATA_DIR='"$(DATADIR)"'

LIBS = -lform -lmenu -lpanel -lncurses $$( [ "$$(uname -s)" = Linux ] && printf '%s' -ltinfo) -lm

SRC		=	src/anim.c src/astro.c src/chronos.c src/conf.c \
			src/draw.c src/indat.c src/init.c src/io.c \
			src/search.c src/ui.c src/zr.c

SWE_SRC = 	swisseph/swedate.c swisseph/swehouse.c swisseph/swejpl.c \
			swisseph/swemmoon.c swisseph/swemplan.c swisseph/sweph.c \
			swisseph/swephlib.c swisseph/swecl.c swisseph/swehel.c
			
SWE_OBJ = 	swisseph/swedate.o swisseph/swehouse.o swisseph/swejpl.o \
			swisseph/swemmoon.o swisseph/swemplan.o swisseph/sweph.o \
			swisseph/swephlib.o swisseph/swecl.o swisseph/swehel.o
			
SWE_D 	= 	swisseph/swedate.d swisseph/swehouse.d swisseph/swejpl.d \
			swisseph/swemmoon.d swisseph/swemplan.d swisseph/sweph.d \
			swisseph/swephlib.d swisseph/swecl.d swisseph/swehel.d

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

install: all
	mkdir -p "$(DESTDIR)$(BINDIR)"
	cp "astro" "$(DESTDIR)$(BINDIR)"
	chmod 755 "$(DESTDIR)$(BINDIR)/astro"
	mkdir -p "$(DESTDIR)$(DATADIR)"
	cp -R "swisseph/ephe" "$(DESTDIR)$(DATADIR)"
	cp city-db "$(DESTDIR)$(DATADIR)"

clean:
	rm $(SWE_OBJ) $(SWE_D) $(SWE_A) astro
	
.PHONY: all debug astro install clean
