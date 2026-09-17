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
// along with this program. if not, see <https://www.gnu.org/lhicenses/>

#pragma once
#include <time.h>

#define NIGHT_SECT 0
#define DAY_SECT 1

#define STATION_R 1
#define STATION_D 2

#define NEXT_E 0
#define PREV_E 1

int sect(struct pxx *pxx);
int daycount(int month, int year);
void eclipse(double jd_ut, double *luna_eclipse, double *sol_eclipse);
void retro_station(double jd_ut, double *planet[]);
void calculate_utc(struct cdata *cdata);
void chart_timeset(struct cdata *cdata, int *day_offset);
void lots(struct pxx *pxx);
void weekday_check(struct cdata *cdata);
void set_localtime(struct cdata *cdata);
void zodiacal_releasing(struct cdata *cdata, struct pxx *pxx, struct ui *ui);
void cpt(struct cdata *cdata, struct tm *temp, struct tm *result, time_t *t, bool x);
