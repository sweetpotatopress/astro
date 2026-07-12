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

#define ERR_EXIT(str) do { \
		endwin(); \
		perror(str); \
		swe_close(); \
		exit(EXIT_FAILURE); \
} while (0)

#define NEW_CHART_MAIN() main_win, planet_win, retro_win, \
		&planet_panel, &retro_panel,\
		io, cdata, pxx,\
		&planet_trig, &retro_trig, cusps, p_arr, z_arr, \
		pl_sym, zo_sym, moon

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
#define MAXPXX 12

#define ARI 1
#define TAU 2
#define GEM 3
#define CAN 4
#define LEO 5
#define VIR 6
#define LIB 7
#define SCO 8
#define SAG 9
#define CAP 10
#define AQU 11
#define PIS 12
#define ZMAX 13

#define M_COLOR 1
#define FIRE 2
#define EARTH 3
#define AIR 4
#define WATER 5

#define ELEMENT 0
#define RULER 1
#define EXALT 2
#define TRIPLD 3
#define TRIPLN 4
#define TRIPLC 5
#define BOUND0 6
#define BOUND1 7
#define BOUND2 8
#define BOUND3 9
#define BOUND4 10
#define DECAN0 11
#define DECAN1 12
#define DECAN2 13
#define DETRI 14
#define FALL 15
#define JOY 16
#define MAXZXX 17

struct zxx {
	int dari[MAXZXX];
	int dtau[MAXZXX];
	int dgem[MAXZXX];
	int dcan[MAXZXX];
	int dleo[MAXZXX];
	int dvir[MAXZXX];
	int dsco[MAXZXX];
	int dsag[MAXZXX];
	int dcap[MAXZXX];
	int daqu[MAXZXX];
	int dpis[MAXZXX];
};

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
