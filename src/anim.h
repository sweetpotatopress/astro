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

void new_chart(NEW_CHART_PARAM());

void animate_chart(NEW_CHART_PARAM());

void realtime_chart(NEW_CHART_PARAM());

void solar_return(NEW_CHART_PARAM());
