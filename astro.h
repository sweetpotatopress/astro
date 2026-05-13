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

#define ERR_EXIT(str) do { \
	endwin(); \
	perror(str); \
	swe_close(); \
	exit(EXIT_FAILURE); \
} while (0)

typedef enum { NORMAL, INSERT } Mode;

#define LONG 0
#define LAT 1
#define DIST 2
#define LONG_S 3
#define LAT_S 4
#define DIST_S 5

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
	double dasc;
	double dmc;
	double ddsc;
	double dic;
	double dfor;
	double dspir;
} Pxx;

// chart data that doesnt fit in struct tm

typedef struct {
	char *city;
	char *citybuffer;
	char *state;
	char *country;
	char *timezone;
	char *latitude;
	char *longitude;
	double dlat;
	double dlon;
	double dhour; //0.0 .. 23.999999;
} Location;

//io

typedef struct {
	char *filepath;
	char *filename;
	size_t file_count;
} Io;

int main_search(FIELD *cdata_field[], char *argv);
void main_io(Io *io, FIELD *cdata_field[], struct tm *cdata,
Location *loc, const char ch);

