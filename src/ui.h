// Copyright (C) 2026 yam lynn
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTIBILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License for more details.

// You should have received a copy of the GNU Affero General Public License
// along with this program. if not, see <https://www.gnu.org/licenses/>
#pragma once

#include <ncurses.h>
#include <panel.h>

#define LWINY 40
#define LWINX 33
#define LWIN_Y 0
#define LWIN_X 0

#define RWINY 9
#define RWINX 24
#define RWIN_Y LINES - 9
#define RWIN_X COLS - 24

#define IWINY 26
#define IWINX 47
#define IWIN_Y (LINES - IWINY) / 2
#define IWIN_X (COLS - IWINX) / 2

#define M_COLOR 1
#define FIRE 2
#define EARTH 3
#define AIR 4
#define WATER 5

#define ELEMENT 0
#define RULER 1
#define EXALT 2
#define TRIPLD 3
#define TRIPLN 4
#define TRIPLC 5
#define BOUND0 6
#define BOUND1 7
#define BOUND2 8
#define BOUND3 9
#define BOUND4 10
#define DECAN0 11
#define DECAN1 12
#define DECAN2 13
#define DETRI 14
#define FALL 15
#define EMPTY 16
#define MAXZXX 17

#define PLMAX 16

#define ARI 1
#define TAU 2
#define GEM 3
#define CAN 4
#define LEO 5
#define VIR 6
#define LIB 7
#define SCO 8
#define SAG 9
#define CAP 10
#define AQU 11
#define PIS 12
#define ZMAX 13

struct zxx {
	int iari[MAXZXX];
	int itau[MAXZXX];
	int igem[MAXZXX];
	int ican[MAXZXX];
	int ileo[MAXZXX];
	int ivir[MAXZXX];
	int ilib[MAXZXX];
	int isco[MAXZXX];
	int isag[MAXZXX];
	int icap[MAXZXX];
	int iaqu[MAXZXX];
	int ipis[MAXZXX];
};

#define PL_SYM_MAX 12
#define ZO_SYM_MAX 13
#define MOON_MAX 8
#define TRANSIT 11
#define MONTH_MAX 13

struct ui_sym {
	const char *pl_sym[PL_SYM_MAX];
	const char *zo_sym[ZO_SYM_MAX];
	const char *moon[MOON_MAX];
	const char *month[MONTH_MAX];
};

struct ui {
	WINDOW *main_win;
	WINDOW *left_win;
	WINDOW *right_win;
	WINDOW *indat_win;
	WINDOW *indat_subwin;
	WINDOW *search_win;
	WINDOW *search_subwin;
	WINDOW *transit_window;
	PANEL *main_panel;
	PANEL *left_panel;
	PANEL *right_panel;
	PANEL *transit_panel;
	int left_trig;
	int right_trig;
	int old_l;
	int old_r;
	int cc; // current chart
	int bcc; // previous chart
	int win_h;
	int win_w;
	int cy; // center y
	int cx; // center x
	int radius;
	int or; // outer radius
	int ir; // inner radius
	int hr; // house radius
	int zr; // zodiac radius
	int pr; // planet radius
	int ar; // angle radius
	struct ui_sym sym;
};

void ui_resize(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac, bool x);
void table_trigger(struct ui *ui, int ch);
void left_table(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac);
void right_table(struct cdata *cdata, struct ui *ui, double **planet, int **zodiac);
