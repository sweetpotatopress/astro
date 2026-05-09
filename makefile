CC        ?= gcc
CFLAGS    ?= -Wall -Wextra -Wpedantic \
             -Wconversion -Wsign-conversion \
             -Wunused-variable -Wunused-function \
             -Wshadow -Wno-implicit-fallthrough \
             -Wredundant-decls -Wfloat-equal \
             -Winline -Wnull-dereference \
             -Waddress -Wno-long-long \
             -Wimplicit-function-declaration \
             -Wno-null-dereference -fanalyzer

SWE_URL   	= https://github.com/aloistr/swisseph/archive/refs/heads/master.tar.gz
SWE_DIR   	= swisseph-master
SWE_INC   	= /usr/local/include
SWE_LIB   	= /usr/local/lib
INSTALL_DIR = /usr/local/bin

TARGET    = astro
SRCS      = astro.c io.c search.c

SWE_HEADERS_EXIST := $(shell test -f $(SWE_INC)/swephexp.h && test -f $(SWE_INC)/sweph.h && echo 1 || echo 0)
SWE_LIB_EXISTS := $(shell test -f $(SWE_LIB)/libswe.a && echo 1 || echo 0)

ifeq ($(SWE_HEADERS_EXIST)$(SWE_LIB_EXISTS),11)
  SWE_DEPS :=
  $(info Swiss Ephemeris found in /usr/local - skipping build)
else
  SWE_DEPS := swe-install
  $(info Swiss Ephemeris not found - will build locally)
endif

.PHONY: all install swe-install swe-clean clean

all: $(SWE_DEPS)
	@echo "-o--o-Building astro -o--/-"
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) \
	     -L$(SWE_LIB) -lswe -lm \
	     -lncurses -lmenu -lform -lpanel -ltinfo

install: all
	@echo "-x--o Installing astro --oo-"
	/bin/mkdir -p $(INSTALL_DIR)
	/bin/cp $(TARGET) $(INSTALL_DIR)/$(TARGET)
	
swe-install: $(SWE_DIR)/libswe.a
	@echo "--o-Installing Swiss Ephemeris x<--o-"
	/bin/mkdir -p $(SWE_INC) $(SWE_LIB)
	/bin/cp $(SWE_DIR)/swephexp.h $(SWE_INC)/swephexp.h
	/bin/cp $(SWE_DIR)/sweph.h     $(SWE_INC)/sweph.h
	/bin/cp $(SWE_DIR)/sweodef.h   $(SWE_INC)/sweodef.h
	/bin/cp $(SWE_DIR)/libswe.a    $(SWE_LIB)/libswe.a

$(SWE_DIR)/libswe.a: $(SWE_DIR)/Makefile
	@echo "--o Building libswe.a -ow0-"
	$(MAKE) -C $(SWE_DIR) libswe.a

$(SWE_DIR)/Makefile: $(SWE_DIR)/.done
	@true

$(SWE_DIR)/.done:
	@echo "-o--Downloading Swiss Ephemeris -owo-"
	rm -rf $(SWE_DIR)
	wget -O swisseph.tar.gz $(SWE_URL)
	@echo "-ox- Extracting Swiss Ephemeris --"
	tar xzf swisseph.tar.gz
	rm -f swisseph.tar.gz
	touch $@

swe-clean:
	rm -rf $(SWE_DIR) swisseph.tar.gz

clean: swe-clean
	rm -f $(TARGET)
