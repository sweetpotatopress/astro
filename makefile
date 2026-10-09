CC	= cc
CFLAGS	:= -Wall -Wextra -Wpedantic -O3
SWE_CFLAGS := -g -Wall -fPIC

PREFIX = /usr/local
LIBS = -lform -lmenu -lpanel -lncurses $$( [ "$$(uname -s)" = Linux ] && printf '%s' -ltinfo) -lm
INC	= -Isrc -Iswisseph

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

SWE_A	= swisseph/libswe.a

CONFIG_DIR	= $(HOME)/.config/astro
DATA_DIR	= $(HOME)/.local/share/astro
CHARTS_DIR	= $(DATA_DIR)/charts

all: data astro

.c.o: $(SWE_SRC)
	$(CC) $(INC) $(SWE_CFLAGS) -MMD -MP -c $< -o $@
-include $(SWE_D)
	
$(SWE_A): $(SWE_OBJ)
	ar rcs $@ $(SWE_OBJ); \
	
astro: ${SWE_A}
	$(CC) $(INC) $(CFLAGS) $(SRC) $(SWE_A) $(LIBS) -o astro

data: 
	mkdir -p "$(CONFIG_DIR)"; \
	mkdir -p "$(CHARTS_DIR)"; \
	cp -r "swisseph/ephe" "$(DATA_DIR)"; \
	cp city-db "$(DATA_DIR)";

install:
	mkdir -p "$(DESTDIR)$(PREFIX)/bin"; \
	cp "astro" "$(DESTDIR)$(PREFIX)/bin"

clean:
	rm $(SWE_OBJ) $(SWE_D)
	
.PHONY: all data astro install clean
