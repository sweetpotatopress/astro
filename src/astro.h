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

#include <ncurses.h>
#include <panel.h>

#define MAXBUF 1024
#define MAXPATH 2048
#define CHARTMAX 12

#define ERR_EXIT(str) do { \
		fprintf(stderr, "%s\n", str); \
		endwin(); \
		swe_close(); \
		exit(EXIT_FAILURE); \
		} while (0)
		
// dst trigger
#define NDST 0
#define YDST 1

// planet data
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
#define DEGREE_S 10
#define MIN_S 11
#define NEXT_S 12
#define NEXT_JUL 13
#define PREV_S 14
#define PREV_JUL 15
#define RET_INIT 16
#define PL_X 17
#define PL_Y 18
#define MAXPXX 19

struct ui;

#define SPXXMAX 18 // struct member count
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

struct cdata {
	char *city;
	char *state;
	char *country;
	char *timezone;
	char *latitude;
	char *longitude;
	char *chart_name;
	double dlat;
	double dlon;
	double utc_hour; //0.0 .. 23.999999;
	double jd_ut;
	int sec;
	int min;
	int hour;
	int mday;
	int mon;
	int year;
	int isdst;
	int wday;
	int utc_year;
	int utc_mon;
	int utc_mday;
	int moonphase;
	double cusp[13];
	double sign_cusp[13];
	double t_cusp; // transit cusp
	double le[EMAX]; // lunar eclipse
	double se[EMAX]; // solar eclipse
};

void *ecalloc(size_t n, size_t size);
void *erealloc(void *p, size_t size);
void config_menu(struct cdata *cdata, struct ui *ui);
void config_init(struct cdata *cdata, struct ui *ui);
void zodiacal_releasing(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac);
