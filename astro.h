/* Copyright (C) 2026 yam lynn
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by the 
Free Software Foundation, either version 3 of the License, or (at your option)
any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTIBILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. if not, see <https://www.gnu.org/licenses/> */

#pragma once
#define ERR_EXIT swe_close(); exit(EXIT_FAILURE);
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

typedef enum { NORMAL, INSERT } Mode;
Mode mode = INSERT;

//planet degrees

typedef struct {
	double dsun;
	double dmoon;
	double dmerc;
	double dven;
	double dmars;
	double djup;
	double dsat;
} P_deg;

//city-search

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
} Location;

int main_search(FIELD *cdata_field[], char *argv);
void save_chart(struct tm *cdata, Location *loc);

