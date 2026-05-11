CC        ?= gcc
CFLAGS    ?= -Wall -Wextra -Wpedantic \
             -Wconversion -Wsign-conversion \
             -Wunused-variable -Wunused-function \
             -Wshadow -Wno-implicit-fallthrough \
             -Wredundant-decls -Wfloat-equal \
             -Winline \
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

# Determine the real user and home directory
REAL_USER := $(shell echo $${SUDO_USER:-$${DOAS_USER:-$$USER}})
REAL_HOME := $(shell getent passwd $(REAL_USER) | cut -d: -f6)

SWE_HEADERS_EXIST := $(shell test -f $(SWE_INC)/swephexp.h && test -f $(SWE_INC)/sweph.h && echo 1 || echo 0)
SWE_LIB_EXISTS := $(shell test -f $(SWE_LIB)/libswe.a && echo 1 || echo 0)
EPHE_EXISTS := $(shell test -d $(REAL_HOME)/.local/share/astro/ephe && echo 1 || echo 0)

ifeq ($(SWE_HEADERS_EXIST)$(SWE_LIB_EXISTS)$(EPHE_EXISTS),111)
  SWE_DEPS :=
  $(info Swiss Ephemeris found - skipping build)
else
  SWE_DEPS := swe-install
  $(info Swiss Ephemeris not found - will build locally)
endif

.PHONY: all install swe-install swe-clean clean

all: $(SWE_DEPS)
	@echo "-o--o-Building astro -o--/-"
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) \
	     -L$(SWE_LIB) -lswe -lm \
	     -lncurses -lmenu -lform -ltinfo
	     
install: all
	@echo "-x--o Installing astro --oo-"
	/bin/mkdir -p $(INSTALL_DIR)
	/bin/cp $(TARGET) $(INSTALL_DIR)/$(TARGET)
	@echo "-x--o Creating data directories --oo-"
	/bin/mkdir -p $(REAL_HOME)/.local/share/astro/charts; \
	/bin/chown -R $(REAL_USER):$(REAL_USER) $(REAL_HOME)/.local/share/astro; \
	/bin/cp -r $(SWE_DIR)/ephe $(REAL_HOME)/.local/share/astro/; \
	/bin/cp city-db $(REAL_HOME)/.local/share/astro/city-db

swe-install: $(SWE_DIR)/libswe.a
	@echo "--o-Installing Swiss Ephemeris x<--o-"
	/bin/mkdir -p $(SWE_INC) $(SWE_LIB)
	/bin/cp $(SWE_DIR)/swephexp.h $(SWE_INC)/swephexp.h
	/bin/cp $(SWE_DIR)/sweph.h     $(SWE_INC)/sweph.h
	/bin/cp $(SWE_DIR)/sweodef.h   $(SWE_INC)/sweodef.h
	/bin/cp $(SWE_DIR)/libswe.a    $(SWE_LIB)/libswe.a
	/bin/mkdir -p $(REAL_HOME)/.local/share/astro/charts; \
	/bin/chown -R $(REAL_USER):$(REAL_USER) $(REAL_HOME)/.local/share/astro; \
	/bin/cp -r $(SWE_DIR)/ephe $(REAL_HOME)/.local/share/astro/; \

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
