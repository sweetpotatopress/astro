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

#include <panel.h>

#define NEW_CHART_ARG() main_win, left_win, right_win, \
	left_panel, right_panel, \
	io, cdata, pxx, \
	left_trig, right_trig, cusp, sign_cusp, planet, zodiac, \
	luna_eclipse, sol_eclipse, \
	pl_sym, zo_sym, moon, cur_chart
	
#define NEW_CHART_PARAM() WINDOW *main_win, WINDOW *left_win, WINDOW *right_win, \
PANEL **left_panel, PANEL **right_panel, \
struct io *io, struct cdata *cdata, struct pxx *pxx, \
int *left_trig, int *right_trig, double cusp[], double sign_cusp[], double *planet[], int *zodiac[], \
double *luna_eclipse, double *sol_eclipse, \
const char *pl_sym[], const char *zo_sym[], const char *moon[], int cur_chart

void cur_chart_data(WINDOW *win, struct io *io, struct cdata *cdata);

void new_chart(NEW_CHART_PARAM());

void animate_chart(NEW_CHART_PARAM());

void realtime_chart(NEW_CHART_PARAM());

void solar_return(NEW_CHART_PARAM());
