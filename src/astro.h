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

#define MAXBUF 1024
#define MAXPATH 2048
#define MAXPXX 7
#define ERR_EXIT(str) do { \
	endwin(); \
	perror(str); \
	swe_close(); \
	exit(EXIT_FAILURE); \
} while (0)

#define PWINY 40
#define PWINX 36
#define PWIN_Y 0
#define PWIN_X 0

#define RWINY 13
#define RWINX 20
#define RWIN_Y (LINES - 10)
#define RWIN_X (COLS - 22)

#define M_COLOR 1
#define FIRE 2
#define EARTH 3
#define AIR 4
#define WATER 5

#define NIGHT_SECT 0
#define DAY_SECT 1

#define CITY 0
#define YEAR 1
#define MONTH 2
#define DAY 3
#define HOUR 4
#define MINUTE 5
#define TIMEZONE 6
#define LATITUDE 7
#define LONGITUDE 8
#define FIELDMAX 9

#define LONG 0
#define LAT 1
#define DIST 2
#define LONG_S 3
#define LAT_S 4
#define DIST_S 5
#define RETRO 6
#define MWIN 7

typedef struct {
	double *dsun;
	double *dmoon;
	double *dmerc;
	double *dven;
	double *dmars;
	double *djup;
	double *dsat;
	double *dura;
	double *dnep;
	double *dplu;
	double *dmnod;
	double *dtnod;
	double *dasc;
	double *dmc;
	double *ddsc;
	double *dic;
	double dfor;
	double dspir;
} Pxx;

typedef struct {
	char *city;
	char *state;
	char *country;
	char *timezone;
	char *latitude;
	char *longitude;
	double dlat;
	double dlon;
	double dhour; //0.0 .. 23.999999;
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_isdst;
	int utc_off;
} Cdata;

typedef struct {
	char *filepath;
	char *filename;
	size_t file_count;
} Io;

typedef enum { NORMAL, INSERT } Mode;

int main_search(FIELD *cdata_field[], char *argv);
void main_io(Io *io, FIELD *cdata_field[], Cdata *cdata, char *citybuffer, const char ch);

int months(int month, int year);
void chart_timeset(Cdata *cdata, int *day_offset);
void pxx_fill(double cusps[], Cdata *cdata, Pxx *pxx);
