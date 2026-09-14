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

void cc_data(WINDOW *win, struct cdata *cdata, struct ui *ui);
void animate_chart(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac);
void realtime_chart(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac);
void solar_return(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac);
void transit(struct cdata **cdata, struct pxx **pxx, struct ui *ui, double **planet, int **zodiac);
void synastry(struct cdata **cdata, struct pxx **pxx, struct ui *ui, double **planet, int **zodiac, int key);
