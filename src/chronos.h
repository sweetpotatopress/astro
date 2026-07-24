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

#define NIGHT_SECT 0
#define DAY_SECT 1

#define RETROCOUNT (SE_PLUTO + 1)
#define STATION_POINT 7

#define STATION_R 1
#define STATION_D 2

void set_localtime(struct cdata *cdata);
int months(int month, int year);
void chart_timeset(struct cdata *cdata,int *day_offset);
void pxx_init(double cusp[], double sign_cusp[], double *luna_eclipse, double *sol_eclipse, double *p_arr[],
struct cdata *cdata, struct pxx *pxx);
int sect(struct pxx *pxx);
