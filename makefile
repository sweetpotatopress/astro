CC        ?= gcc
CFLAGS    ?= -Wall -Wextra -Wpedantic \
             -Wconversion -Wsign-conversion \
             -Wdouble-promotion -Wtype-limits \
             -Wold-style-declaration \
             -Wformat-security -Wformat-nonliteral \
             -Wjump-misses-init -Wuninitialized \
             -Wmissing-field-initializers \
             -Wunused-variable -Wunused-function -Wunused-parameter \
             -Wshadow -Wno-implicit-fallthrough \
             -Wredundant-decls -Wfloat-equal \
             -Winline -Waddress \
             -Wno-long-long \
             -Wimplicit-function-declaration \
             -Wno-null-dereference -fanalyzer

SWE_DIR   	= swisseph
SWE_INC   	= /usr/local/include
SWE_LIB   	= /usr/local/lib
INSTALL_DIR = /usr/local/bin

TARGET    = astro
SRCS      = astro.c io.c search.c

# Determine the real user and home directory
REAL_USER := $(shell echo $${SUDO_USER:-$${DOAS_USER:-$$USER}})
REAL_HOME := $(shell getent passwd $(REAL_USER) | cut -d: -f6)

# Check if Swiss Ephemeris is installed system-wide
SWE_HEADERS_EXIST := $(shell test -f $(SWE_INC)/swephexp.h && test -f $(SWE_INC)/sweph.h && test -f $(SWE_INC)/sweodef.h && echo 1 || echo 0)
SWE_LIB_EXISTS := $(shell test -f $(SWE_LIB)/libswe.a && echo 1 || echo 0)
EPHE_EXISTS := $(shell test -d $(REAL_HOME)/.local/share/astro/ephe && echo 1 || echo 0)

ifeq ($(SWE_HEADERS_EXIST)$(SWE_LIB_EXISTS)$(EPHE_EXISTS),111)
  SWE_DEPS :=
  $(info Swiss Ephemeris found - using them uwu)
else
  SWE_DEPS := swe-install
  $(info Swiss Ephemeris not found - installing --o-i)
endif

.PHONY: all install swe-install clean

all: $(SWE_DEPS)
	@echo "-o--o-Building astro -o--/-"
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) \
	    -L$(SWE_LIB) -lswe -lm \
	    -lpanel -lmenu -lform -lncurses -ltinfo

install: all
	@echo "-x--o Installing astro --oo-"
	/bin/mkdir -p $(INSTALL_DIR)
	/bin/cp $(TARGET) $(INSTALL_DIR)/$(TARGET)
	@echo "-x--o Creating data directories --oo-"
	/bin/mkdir -p $(REAL_HOME)/.local/share/astro/charts; \
	/bin/chown -R $(REAL_USER):$(REAL_USER) $(REAL_HOME)/.local/share/astro; \
	/bin/cp -r $(SWE_DIR)/ephe $(REAL_HOME)/.local/share/astro/; \
	/bin/cp city-db $(REAL_HOME)/.local/share/astro/city-db

swe-install:
	@echo "--o-Installing Swiss Ephemeris x<--o-"
	/bin/mkdir -p $(SWE_INC) $(SWE_LIB)
	/bin/cp $(SWE_DIR)/swephexp.h $(SWE_INC)/swephexp.h
	/bin/cp $(SWE_DIR)/sweph.h     $(SWE_INC)/sweph.h
	/bin/cp $(SWE_DIR)/sweodef.h   $(SWE_INC)/sweodef.h
	/bin/cp $(SWE_DIR)/libswe.a    $(SWE_LIB)/libswe.a
	/bin/mkdir -p $(REAL_HOME)/.local/share/astro/charts; \
	/bin/chown -R $(REAL_USER):$(REAL_USER) $(REAL_HOME)/.local/share/astro; \
	/bin/cp -r $(SWE_DIR)/ephe $(REAL_HOME)/.local/share/astro/

clean:
	rm -f $(TARGET)
