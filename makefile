CC        ?= gcc
CFLAGS    ?= -Wall -Wextra -Wpedantic -Isrc -Iswisseph \
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
             
SWE_CFLAGS	= -g -Wall -fPIC

TARGET		= astro
SRCS		= $(wildcard src/*.c)
INSTALL_DIR = /usr/local/bin

SWE_DIR     = swisseph

SWE_SRCS	:= $(wildcard swisseph/*.c)
SWE_OBJS	:= $(patsubst swisseph/%.c,swisseph/%.o,$(SWE_SRCS))
SWE_A		:= $(SWE_DIR)/libswe.a

SWE_LIB_EXISTS := $(shell test -f $(SWE_DIR)/libswe.a && echo 1 || echo 0)

REAL_USER := $(shell echo $${SUDO_USER:-$${DOAS_USER:-$$USER}})
REAL_HOME := $(shell getent passwd $(REAL_USER) | cut -d: -f6)

XDG_CONFIG_HOME := $(shell \
  if [ -n "$${XDG_CONFIG_HOME}" ]; then printf "%s" "$${XDG_CONFIG_HOME}"; \
  else printf "%s/.config" "$(REAL_HOME)"; fi)

XDG_DATA_HOME := $(shell \
  if [ -n "$${XDG_DATA_HOME}" ]; then printf "%s" "$${XDG_DATA_HOME}"; \
  else printf "%s/.local/share" "$(REAL_HOME)"; fi)

CONFIG_DIR	:= $(XDG_CONFIG_HOME)/astro
DATA_DIR    := $(XDG_DATA_HOME)/astro
CHARTS_DIR  := $(DATA_DIR)/charts
EPHE_DIR    := $(DATA_DIR)/ephe

SWE_DEPS :=
ifeq ($(SWE_LIB_EXISTS)$(shell test -d "$(EPHE_DIR)" && echo 1 || echo 0),11)
  SWE_DEPS :=
  $(info libswe.a found: $(SWE_A))
else
  SWE_DEPS := swe-install
  $(info libswe.a not found - building --o-i)
endif

.PHONY: all install swe-install clean debug

all: $(SWE_DEPS)
	@echo "-o--o-building astro -o--/-"
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) \
	    -L$(SWE_DIR) -lswe -lm \
	    -lpanel -lmenu -lform -lncurses -ltinfo

debug: $(SWE_DEPS)
	@echo "-o--o-debug build (sanitizers) --o--/-"
	$(CC) \
	  $(CFLAGS) \
	  -g3 -fno-omit-frame-pointer \
	  -fsanitize=undefined,address,leak,bounds \
	  -fno-sanitize-recover=undefined \
	  -o $(TARGET) $(SRCS) \
	  -L$(SWE_DIR) -lswe -lm \
	  -lpanel -lmenu -lform -lncurses -ltinfo

$(SWE_DIR)/%.o: swisseph/%.c
	$(CC) $(SWE_CFLAGS) -c $< -o $@
		
$(SWE_A): $(SWE_OBJS)
	/bin/ar rcs $@ $(SWE_OBJS)
	/bin/rm -f $(SWE_OBJS)

install: all
	@echo "-x--o installing astro --oo-"
	/bin/mkdir -p "$(INSTALL_DIR)"
	/bin/cp "$(TARGET)" "$(INSTALL_DIR)/$(TARGET)"

	@echo "-x--o creating data directories --oo-"
	/bin/mkdir -p "$(CONFIG_DIR)"; \
	/bin/mkdir -p "$(CHARTS_DIR)"; \
	/bin/chown -R $(REAL_USER):$(REAL_USER) "$(CONFIG_DIR)"; \
	/bin/chown -R $(REAL_USER):$(REAL_USER) "$(DATA_DIR)"

	/bin/cp -r "$(SWE_DIR)/ephe" "$(DATA_DIR)/"; \
	/bin/cp city-db "$(DATA_DIR)/city-db"

swe-install: $(SWE_A)
	@echo "--o-installing swiss ephemeris x<--o-"

	/bin/mkdir -p "$(CHARTS_DIR)" "$(EPHE_DIR)"; \
	/bin/chown -R $(REAL_USER):$(REAL_USER) "$(DATA_DIR)"; \
	/bin/cp -r "$(SWE_DIR)/ephe" "$(DATA_DIR)/"

clean:
	rm -f "$(TARGET)"
