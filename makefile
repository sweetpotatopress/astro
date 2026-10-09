CFLAGS	:= -Wall -Wextra -Wpedantic -O3
SWE_CFLAGS := -g -Wall -fPIC

TARGET = astro
INSTALL_DIR = /usr/local/bin
LIBS = -lform -lmenu -lpanel -lncurses \
$$( [ "$$(uname -s)" = Linux ] && printf '%s' -ltinfo) -lm

INC = -Isrc -Iswisseph
SRCS  = src/anim.c src/astro.c src/chronos.c src/conf.c \
		src/draw.c src/indat.c src/init.c src/io.c \
		src/search.c src/ui.c src/zr.c

SWE_SRCS = swisseph/swedate.c swisseph/swehouse.c swisseph/swejpl.c \
			swisseph/swemmoon.c swisseph/swemplan.c swisseph/sweph.c \
			swisseph/swephlib.c swisseph/swecl.c swisseph/swehel.c
			
SWE_OBJS = swisseph/swedate.o swisseph/swehouse.o swisseph/swejpl.o \
			swisseph/swemmoon.o swisseph/swemplan.o swisseph/sweph.o \
			swisseph/swephlib.o swisseph/swecl.o swisseph/swehel.o

SWE_DIR	= swisseph
SWE_A := $(SWE_DIR)/libswe.a
CONFIG_DIR	= $(HOME)/.config/astro
DATA_DIR	= $(HOME)/.local/share/astro
CHARTS_DIR	= $(DATA_DIR)/charts

.PHONY: all debug install clean

all: ${SWE_A}
	mkdir -p "$(CONFIG_DIR)"; \
	mkdir -p "$(CHARTS_DIR)"; \
	cp -r "$(SWE_DIR)/ephe" "$(DATA_DIR)"; \
	cp city-db "$(DATA_DIR)"; \
	cc $(INC) $(CFLAGS) $(SRCS) $(SWE_A) $(LIBS) -o $(TARGET); \
	rm $(SWE_OBJS)
	
debug:
	cc $(INC) $(CFLAGS) -g -O0 -fno-omit-frame-pointer \
	$(SRCS) $(SWE_A) $(LIBS)
	
.c.o:
	cc $(INC) $(SWE_CFLAGS) -c $< -o $@

$(SWE_A): $(SWE_OBJS)
	ar rcs $@ $(SWE_OBJS)
	
install:
	mkdir -p "$(INSTALL_DIR)"; \
	cp "$(TARGET)" "$(INSTALL_DIR)"
