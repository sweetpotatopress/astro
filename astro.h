#pragma once

typedef struct {
	char *city;
	char *country;
	char *latitude;
	char *longitude;
} Location;

int n_choices = 0;

int main_search(int argc, char *argv);
int city_search(FILE *ifp, char *search, Location **choices, int max_choices);
void location_to_string(Location *loc, char *buffer, int buffer_size);
void print_menu(WINDOW *menu_win, int highlight, Location **choices);
