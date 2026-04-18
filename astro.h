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

//chart data
typedef struct {
	int iyar;
	int imon;
	int iday;
	double dhour;
	double dlon;
	double dlat;
} Cdata;

//city-search
typedef struct {
	char *city;
	char *country;
	char *latitude;
	char *longitude;
} Location;

int n_choices = 0;

int main_search(char *argv);
int location_search_parse(FILE *ifp, char *search, Location **choices, int max_choices);
void print_menu(WINDOW *menu_win, int highlight, Location **choices);
