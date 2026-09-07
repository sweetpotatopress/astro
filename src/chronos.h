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

#define NIGHT_SECT 0
#define DAY_SECT 1

#define STATION_R 1
#define STATION_D 2

#define NEXT_E 0
#define PREV_E 1

#define E_INIT 0
#define EN_JUL 1
#define EN_FJUL 2
#define EN_OBS 3
#define EN_SIGN 4
#define EP_JUL 5
#define EP_FJUL 6
#define EP_OBS 7
#define EP_SIGN 8
#define EMAX 9

int sect(struct pxx *pxx);
int leapyear(int month, int year);
void eclipse(double jd_ut, double *luna_eclipse, double *sol_eclipse);
void retro_station(double jd_ut, double *planet[]);
void calculate_utc(struct cdata *cdata);
void chart_timeset(struct cdata *cdata, int *day_offset);
void lots(struct pxx *pxx);
void weekday_check(struct cdata *cdata);
void set_localtime(struct cdata *cdata);
