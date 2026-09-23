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
