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

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <menu.h>
 
size_t location_search_parse(FILE *ifp, char *search, Location **choices)
{
	size_t i = 0;
	char buffer[1024] = {0};
	
	while (fgets(buffer, sizeof(buffer), ifp) != NULL)
	{
		size_t len = strlen(buffer);
		if (buffer[len - 1] == '\n')
			buffer[len - 1] = '\0';
		
		char *copy = calloc(1, strlen(buffer) + 1);
		if (!copy)
		{
			perror("copy malloc");
			ERR_EXIT;
		}
		strcpy(copy, buffer);
		
		char *token = strtok(copy, "\t");
		int field_count = 0;
		char *fields[19] = {0};
				
		while (token != NULL && field_count < 19)
		{
			fields[field_count] = calloc(1, strlen(token) + 1);
			strcpy(fields[field_count], token);
			field_count++;
			token = strtok(NULL, "\t");
		}
		
		if (field_count > 1 && strcasestr(fields[1], search) != NULL)
		{
			Location *location = calloc(1, sizeof(Location));
			location->city = fields[2];
			location->state = fields[9];
			location->country = fields[8];
			location->timezone = fields[14];
			location->latitude = fields[4];
			location->longitude = fields[5];
			choices[i++] = location;
		}
		free(copy);
	}
	return i;
}

void print_menu(Location **choices)
{
	int c;
	ITEM **cities;
	MENU *city_menu;
	char buffer[256];
	cities = calloc(n_choices + 1, sizeof(ITEM *));
	if (!cities)
	{
		perror("cities calloc");
		ERR_EXIT;
	}
	// stores the combined_location pointer in the below loop
	// to be freed later
	char **freecombined = malloc (n_choices * sizeof(char *));
	if (!freecombined)
	{
		perror("freecombined malloc");
		ERR_EXIT;
	}

	for (size_t i = 0; i < n_choices; ++i)
	{
		snprintf(buffer, sizeof(buffer), "%-25.25s %.2s %-10s %-5s %-5s %s",
			choices[i]->city,
			choices[i]->state,
			choices[i]->country,
			choices[i]->timezone,
			choices[i]->latitude,
			choices[i]->longitude);
		
	char *combined_location = calloc(1, strlen(buffer) + 1);
		if(!combined_location)
		{
			perror("combined location malloc");
			ERR_EXIT;
		}
	
		strcpy(combined_location, buffer);
		freecombined[i] = combined_location;
		cities[i] = new_item(combined_location, NULL);
	}
	cities[n_choices] = NULL;

	city_menu = new_menu((ITEM **)cities);	
	if (city_menu == NULL) 
	{
		fprintf(stderr, "ERROR: new_menu failed!");
		getch();
		endwin();
		free(freecombined);
		return;
	}
	menu_opts_off(city_menu, O_NONCYCLIC);
	
	int post_result = post_menu(city_menu);
	if (post_result != E_OK)
	{
		fprintf(stderr, "ERROR: post_menu failed!, code %d", post_result);
		getch();
		endwin();
		free(freecombined);
		return;
	}
	
	refresh();

	while((c = getch()) != KEY_F(1))
	{
		switch(c)
		{
			case 'j':
				menu_driver(city_menu, REQ_DOWN_ITEM);
				break;
			case 'k':
				menu_driver(city_menu, REQ_UP_ITEM);
				break;
			case '\n':
				menu_driver(city_menu, REQ_TOGGLE_ITEM);
				break;
		}
	}
	
	unpost_menu(city_menu);
	free_menu(city_menu);
	free(cities);
	free(freecombined);
}
		

int main_search(char *argv)
{
	FILE *fp;
	const char *path = "cities";
	char *search = argv;
	Location **choices = calloc(1, sizeof(Location *) * 100);
	
	
	fp = fopen(path, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "can't open %s\n", path);
		free(choices);
		getch();
		endwin();
		exit(1);
	}
	
	initscr();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);

	n_choices = location_search_parse(fp, search, choices);
	
	if (n_choices >= 100)
	{
		fprintf(stderr, "too many results, be more precise\n");
		getch();
		n_choices = location_search_parse(fp, search, choices);
	}
	
	if (n_choices == 0)
	{
		fprintf(stderr, "no search results\n");
		getch();
		n_choices = location_search_parse(fp, search, choices);
	}
	
	print_menu(choices);
	
	clear();
	refresh();
	fclose(fp);
	free(choices);
	endwin();
	return 0;
}
