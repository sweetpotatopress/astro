CFLAGS	:= -Wall -Wextra -Wpedantic -O3 -std=c99
SWE_CFLAGS = -g -Wall -fPIC -std=c99

SWE_DIR	= swisseph
SWE_A := $(SWE_DIR)/libswe.a
SWE_SRCS != echo $(SWE_DIR)/*.c
SWE_OBJS = $(SWE_SRCS:S,.c,.o,)

LIBS = -lform -lmenu -lpanel -lncurses -lm
INC = -Isrc -Iswisseph
SRCS != echo src/*.c
TARGET = astro
INSTALL_DIR = /usr/local/bin

CONFIG_DIR	= $(HOME)/.config/astro
DATA_DIR	= $(HOME)/.local/share/astro
CHARTS_DIR	= $(DATA_DIR)/charts
EPHE_DIR	= $(DATA_DIR)/ephe

.PHONY: all

all: ${SWE_A}
	mkdir -p "$(CONFIG_DIR)"; \
	mkdir -p "$(CHARTS_DIR)"; \
	mkdir -p "$(EPHE_DIR)"; \
	cp -r "$(SWE_DIR)/ephe" "$(DATA_DIR)"; \
	cp city-db "$(DATA_DIR)"; \
	cc $(INC) $(CFLAGS) $(SRCS) $(SWE_A) $(LIBS) -o $(TARGET)
	
debug:
	cc $(INC) $(CFLAGS) -g -O0 -fno-omit-frame-pointer \
	$(SRCS) $(SWE_A) $(LIBS)

.for obj in $(SWE_OBJS)
$(obj): $(obj:R).c
	cc $(INC) $(SWE_CFLAGS) -c ${.IMPSRC} -o ${.TARGET}
.endfor

$(SWE_A): $(SWE_OBJS)
	ar rcs $(.TARGET) $(SWE_OBJS)
	
install:
	mkdir -p "$(INSTALL_DIR)"
	cp "$(TARGET)" "$(INSTALL_DIR)/$(TARGET)"
