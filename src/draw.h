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

void draw_chart(WINDOW *win, double cusp[], double sign_cusp[], double *p_arr[], int *z_arr[],
struct pxx *pxx, struct cdata *cdata,
const char *pl_sym[], const char *zo_sym[]);

void cur_chart_data(WINDOW *win, struct io *io, struct cdata *cdata);

void zodiac_color(WINDOW *win, int y, int x, int sign,
const char *zo_sym[], int *z_arr[]);

void degree_color(WINDOW *win, int y, int x, int count,
double *p_arr[], int *z_arr[]);


