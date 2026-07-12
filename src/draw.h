/* Copyright (C) 2026 yam lynn
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License
as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTIBILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. if not, see <https://www.gnu.org/licenses/> */
 
#include <ncurses.h>
#include <panel.h>

#define NEW_CHART_ARG() main_win, planet_win, retro_win, \
	planet_panel, retro_panel, \
	io, cdata, pxx, \
	planet_trig, retro_trig, cusps, p_arr, z_arr, \
	pl_sym, zo_sym, moon
	
#define NEW_CHART_PARAM() WINDOW *main_win, WINDOW *planet_win, WINDOW *retro_win, \
PANEL **planet_panel, PANEL **retro_panel, \
struct io *io, struct cdata *cdata, struct pxx *pxx, \
int *planet_trig, int *retro_trig, double cusps[], double *p_arr[], int *z_arr[], \
const char *pl_sym[], const char *zo_sym[], const char *moon[] 

#define PWINY 40
#define PWINX 33
#define PWIN_Y 0
#define PWIN_X 0

#define RWINY 9
#define RWINX 24
#define RWIN_Y LINES - 9
#define RWIN_X COLS - 24

#define CWINY 20
#define CWINX 47
#define CWIN_Y (LINES - CWINY) / 2
#define CWIN_X (COLS - CWINX) / 2

void draw_chart(WINDOW *main_win, double cusps[], double *p_arr[], int *z_arr[],
struct pxx *pxx, struct cdata *cdata,
const char *pl_sym[], const char *zo_sym[]);
void cur_chart_data(WINDOW *main_win, struct io *io, struct cdata *cdata);

void planet_table(WINDOW *planet_win, double *p_arr[], int *z_arr[], struct pxx *pxx,
const char *pl_sym[], const char *zo_sym[], const char *moon[]);
void retro_table(WINDOW *retro_win, double *p_arr[], 
const char *pl_sym[]);

void new_chart(NEW_CHART_PARAM());

void animate_chart(NEW_CHART_PARAM());

void realtime_chart(NEW_CHART_PARAM());

void solar_return(NEW_CHART_PARAM());
