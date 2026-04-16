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

typedef struct {
	char *city;
	char *country;
	char *latitude;
	char *longitude;
} Location;

int n_choices = 0;

void main_search(char *argv);
int city_search(FILE *ifp, char *search, Location **choices, int max_choices);
void location_to_string(Location *loc, char *buffer, int buffer_size);
void print_menu(WINDOW *menu_win, int highlight, Location **choices);
