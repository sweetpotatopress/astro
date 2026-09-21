CC        ?= gcc
CFLAGS    ?= -Wall -Wextra -Wpedantic -Isrc -Iswisseph \
             -Wconversion -Wsign-conversion \
             -Wdouble-promotion -Wtype-limits \
             -Wold-style-declaration \
             -Wformat-security -Wformat-nonliteral \
             -Wuninitialized -Wmissing-field-initializers \
             -Wunused-variable -Wunused-function -Wunused-parameter \
             -Wshadow -Wno-implicit-fallthrough -Werror=implicit-function-declaration \
             -Wredundant-decls -Wfloat-equal \
             -Wnull-dereference -Waddress \
             -O3 -std=c99 -D_POSIX_C_SOURCE=200809L
             
TARGET		= astro
SRCS		= $(wildcard src/*.c)
INSTALL_DIR = /usr/local/bin

SWE_CFLAGS	= -g -Wall -fPIC -std=c99 -D_POSIX_C_SOURCE=200809L
SWE_DIR     = swisseph
SWE_BUILD	:= $(SWE_DIR)/build

SWE_SRCS	:= $(wildcard $(SWE_DIR)/*.c)
SWE_OBJS	:= $(patsubst $(SWE_DIR)/%.c,$(SWE_BUILD)/%.o,$(SWE_SRCS))
SWE_D		:= $(SWE_OBJS:.o=.d)
SWE_A		:= $(SWE_DIR)/libswe.a

REAL_USER := $(shell echo $${SUDO_USER:-$${DOAS_USER:-$$USER}})
REAL_HOME := $(shell getent passwd $(REAL_USER) | cut -d: -f6)

XDG_CONFIG_HOME := $(or $(XDG_CONFIG_HOME),$(REAL_HOME)/.config)
XDG_DATA_HOME	:= $(or $(XDG_DATA_HOME),$(REAL_HOME)/.local/share)

CONFIG_DIR	:= $(XDG_CONFIG_HOME)/astro
DATA_DIR    := $(XDG_DATA_HOME)/astro
CHARTS_DIR  := $(DATA_DIR)/charts
EPHE_DIR    := $(DATA_DIR)/ephe

SWE_A_EXISTS := $(if $(wildcard $(SWE_A)),1,0)
SWE_EPHE_EXISTS := $(if $(wildcard $(EPHE_DIR)/.),1,0)

SWE_DEPS :=
ifeq ($(SWE_A_EXISTS)$(SWE_EPHE_EXISTS),11)
  SWE_DEPS :=
  $(info --o $(SWE_A) and $(EPHE_DIR) found --x-)
else
  SWE_DEPS := swe-install
  $(info --o $(SWE_A) and/or $(EPHE_DIR) not found, building --x-)
endif

.PHONY: all install swe-install clean debug

all: $(SWE_DEPS) $(SWE_A)
	@echo "-o--o-building astro -o--/-"
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) \
	    -L$(SWE_BUILD) -lswe -lm \
	    -lpanel -lmenu -lform -lncurses -ltinfo

debug: $(SWE_DEPS)
	@echo "-o--o-debug build --o--/-"
	$(CC) \
	  $(CFLAGS) \
	  -g3 -fno-omit-frame-pointer -fanalyzer \
	  -fsanitize=undefined,address,leak,bounds \
	  -fno-sanitize-recover=undefined \
	  -o $(TARGET) $(SRCS) \
	  -L$(SWE_DIR) -lswe -lm \
	  -lpanel -lmenu -lform -lncurses -ltinfo

$(SWE_BUILD)/%.o: $(SWE_DIR)/%.c
	$(CC) $(SWE_CFLAGS) -MMD -MP -c $< -o $@
	
-include $(SWE_D)
		
$(SWE_A): $(SWE_OBJS)
	ar rcs $@ $(SWE_OBJS)

install: all
	@echo "-x--o installing astro --oo-"
	mkdir -p "$(INSTALL_DIR)"
	cp "$(TARGET)" "$(INSTALL_DIR)/$(TARGET)"

	@echo "-x--o creating data directories --oo-"
	mkdir -p "$(CONFIG_DIR)"; \
	mkdir -p "$(CHARTS_DIR)"; \
	chown -R $(REAL_USER):$(REAL_USER) "$(CONFIG_DIR)"; \
	chown -R $(REAL_USER):$(REAL_USER) "$(DATA_DIR)"

	cp -r "$(SWE_DIR)/ephe" "$(DATA_DIR)/"; \
	cp city-db "$(DATA_DIR)/city-db"

swe-install: $(SWE_A)
	@echo "--o-installing swiss ephemeris x<--o-"
	mkdir -p "$(CHARTS_DIR)" "$(EPHE_DIR)"; \
	chown -R $(REAL_USER):$(REAL_USER) "$(DATA_DIR)"; \
	cp -r "$(SWE_DIR)/ephe" "$(DATA_DIR)/"

clean:
	rm -f "$(TARGET)"
