CC        ?= gcc
CFLAGS    ?= -Wall -Wextra -Wpedantic -Isrc \
             -Wconversion -Wsign-conversion \
             -Wdouble-promotion -Wtype-limits \
             -Wold-style-declaration \
             -Wformat-security -Wformat-nonliteral \
             -Wjump-misses-init -Wuninitialized \
             -Wmissing-field-initializers \
             -Wunused-variable -Wunused-function -Wunused-parameter \
             -Wshadow -Wno-implicit-fallthrough \
             -Wredundant-decls -Wfloat-equal \
             -Wnull-dereference \
             -Waddress -Wimplicit-function-declaration \
             -fanalyzer -O3

SWE_DIR     = swisseph
SWE_INC     = /usr/local/include
SWE_LIB     = /usr/local/lib
INSTALL_DIR = /usr/local/bin

TARGET     = astro
SRCS       = $(wildcard src/*.c)

REAL_USER := $(shell echo $${SUDO_USER:-$${DOAS_USER:-$$USER}})
REAL_HOME := $(shell getent passwd $(REAL_USER) | cut -d: -f6)

SWE_HEADERS_EXIST := $(shell test -f $(SWE_INC)/swephexp.h && test -f $(SWE_INC)/sweph.h && test -f $(SWE_INC)/sweodef.h && echo 1 || echo 0)
SWE_LIB_EXISTS := $(shell test -f $(SWE_LIB)/libswe.a && echo 1 || echo 0)

XDG_CONFIG_HOME := $(shell \
  if [ -n "$${XDG_CONFIG_HOME}" ]; then printf "%s" "$${XDG_CONFIG_HOME}"; \
  else printf "%s/.config" "$(REAL_HOME)"; fi)

XDG_DATA_HOME := $(shell \
  if [ -n "$${XDG_DATA_HOME}" ]; then printf "%s" "$${XDG_DATA_HOME}"; \
  else printf "%s/.local/share" "$(REAL_HOME)"; fi)

CONFIG_DIR := $(XDG_CONFIG_HOME)/astro
DATA_DIR    := $(XDG_DATA_HOME)/astro
CHARTS_DIR  := $(DATA_DIR)/charts
EPHE_DIR    := $(DATA_DIR)/ephe

SWE_DEPS :=
ifeq ($(SWE_HEADERS_EXIST)$(SWE_LIB_EXISTS)$(shell test -d "$(EPHE_DIR)" && echo 1 || echo 0),111)
  SWE_DEPS :=
  $(info Swiss Ephemeris found - using them uwu)
else
  SWE_DEPS := swe-install
  $(info Swiss Ephemeris not found - installing --o-i)
endif

.PHONY: all install swe-install clean debug

all: $(SWE_DEPS)
	@echo "-o--o-Building astro -o--/-"
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) \
	    -L$(SWE_LIB) -lswe -lm \
	    -lpanel -lmenu -lform -lncurses -ltinfo

debug: $(SWE_DEPS)
	@echo "-o--o-Debug build (sanitizers) --o--/-"
	$(CC) \
	  $(CFLAGS) \
	  -g3 -O1 -fno-omit-frame-pointer \
	  -fsanitize=undefined,address,leak,bounds \
	  -fno-sanitize-recover=undefined \
	  -o $(TARGET) $(SRCS) \
	  -L$(SWE_LIB) -lswe -lm \
	  -lpanel -lmenu -lform -lncurses -ltinfo

install: all
	@echo "-x--o Installing astro --oo-"
	/bin/mkdir -p "$(INSTALL_DIR)"
	/bin/cp "$(TARGET)" "$(INSTALL_DIR)/$(TARGET)"

	@echo "-x--o Creating data directories --oo-"
	/bin/mkdir -p "$(CONFIG_DIR)"; \
	/bin/mkdir -p "$(CHARTS_DIR)"; \
	/bin/chown -R $(REAL_USER):$(REAL_USER) "$(CONFIG_DIR)"; \
	/bin/chown -R $(REAL_USER):$(REAL_USER) "$(DATA_DIR)"

	/bin/cp -r "$(SWE_DIR)/ephe" "$(DATA_DIR)/"; \
	/bin/cp city-db "$(DATA_DIR)/city-db"

swe-install:
	@echo "--o-Installing Swiss Ephemeris x<--o-"
	/bin/mkdir -p "$(SWE_INC)" "$(SWE_LIB)"
	/bin/cp "$(SWE_DIR)/swephexp.h" "$(SWE_INC)/swephexp.h"
	/bin/cp "$(SWE_DIR)/sweph.h"     "$(SWE_INC)/sweph.h"
	/bin/cp "$(SWE_DIR)/sweodef.h"   "$(SWE_INC)/sweodef.h"
	/bin/cp "$(SWE_DIR)/libswe.a"    "$(SWE_LIB)/libswe.a"

	/bin/mkdir -p "$(CHARTS_DIR)" "$(EPHE_DIR)"; \
	/bin/chown -R $(REAL_USER):$(REAL_USER) "$(DATA_DIR)"; \
	/bin/cp -r "$(SWE_DIR)/ephe" "$(DATA_DIR)/"

clean:
	rm -f "$(TARGET)"
