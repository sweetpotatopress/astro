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
#pragma once

#include <swephexp.h>
#include <time.h>
#include <form.h>

#define MAXBUF 1024
#define MAXPATH 2048
#define MAXPXX 12

#define ERR_EXIT(str) do { \
	endwin(); \
	perror(str); \
	swe_close(); \
	exit(EXIT_FAILURE); \
} while (0)

#define NEW_CHART_MAIN() main_win, planet_win, retro_win, \
					&planet_panel, &retro_panel,\
					io, cdata, pxx,\
					&planet_trig, &retro_trig, cusps, p_arr, \
					pl_sym, zo_sym, moon

#define PWINY 40
#define PWINX 33
#define PWIN_Y 0
#define PWIN_X 0

#define RWINY 9
#define RWINX 24
#define RWIN_Y LINES - 9
#define RWIN_X COLS - 24

#define M_COLOR 1
#define FIRE 2
#define EARTH 3
#define AIR 4
#define WATER 5

#define CWINY 20
#define CWINX 47
#define CWIN_Y (LINES - CWINY) / 2
#define CWIN_X (COLS - CWINX) / 2

#define LONG 0
#define LAT 1
#define DIST 2
#define LONG_S 3
#define LAT_S 4
#define DIST_S 5
#define RETRO 6
#define STATION 7
#define DEGREE 8
#define MIN 9
#define NEXT_S 10
#define NEXT_R 11

struct pxx {
	double dsun[MAXPXX];
	double dmoon[MAXPXX];
	double dmerc[MAXPXX];
	double dven[MAXPXX];
	double dmars[MAXPXX];
	double djup[MAXPXX];
	double dsat[MAXPXX];
	double dura[MAXPXX];
	double dnep[MAXPXX];
	double dplu[MAXPXX];
	double dmnod[MAXPXX];
	double dtnod[MAXPXX];
	double dasc[MAXPXX];
	double dmc[MAXPXX];
	double ddsc[MAXPXX];
	double dic[MAXPXX];
	double dfor[MAXPXX];
	double dspir[MAXPXX];
};

struct cdata {
	char *city;
	char *state;
	char *country;
	char *timezone;
	char *latitude;
	char *longitude;
	double dlat;
	double dlon;
	double utc_hour; //0.0 .. 23.999999;
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_isdst;
	int utc_year;
	int utc_mon;
	int utc_mday;
};

struct io {
	char *filepath;
	char *filename;
	size_t file_count;
};

enum mode { NORMAL, INSERT };

void city_search(FIELD *cdata_field[], FORM *cdata_form, char *search,
char *statebuffer, char *countrybuffer);

void load_chart(struct cdata *cdata, struct io *io,
char *citybuffer, char *statebuffer, char *countrybuffer);
void save_chart(struct cdata *cdata, struct io *io,
char *citybuffer, char *statebuffer, char *countrybuffer);
