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

void cdata_clear(struct cdata *cdata);
void ecst_init(double *planet[], double *sol_eclipse);
void planet_init(double *planet[], int cur_chart, struct pxx **pxx);
void zxx_init(int *zodiac[]);
void pxx_init(struct cdata *cdata, struct pxx *pxx, double **planet);
void new_chart(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac);
