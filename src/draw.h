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

#include <panel.h>

void zo_color(WINDOW *win, struct ui *ui, int y, int x, int sign, int *zodiac[]);
void degree_color(WINDOW *win, int y, int x, int count, double *planet[], int *zodiac[]);
void planet_pos(WINDOW *win, struct cdata *cdata, struct ui *ui, double **planet, int **zodiac, int radius, int cy, int cx);
void ascmc_pos(WINDOW *win, struct cdata *cdata, double **planet, int **zodiac, int radius, int cy, int cx);
void zo_pos(WINDOW *win, struct cdata *cdata, struct pxx *pxx, struct ui *ui, int radius, int cy, int cx, int **zodiac);
void draw_house(WINDOW *win, struct cdata *cdata, int radius, int cy, int cx, chtype ch);
void draw_circle(WINDOW *win, int radius, int cy, int cx, chtype ch);

