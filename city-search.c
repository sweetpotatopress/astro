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

char* strtok_E(char *str, const char *delim)
{
	static char *next_pos = NULL;
	char *token_start;
	static char empty_token[] = "E";
	
	if (str != NULL)
		next_pos = str;
		
	if (next_pos == NULL || *next_pos == '\0')
		return NULL;
		
	if (strchr(delim, *next_pos) != NULL)
	{
		next_pos++;
		return empty_token;
	}
	
	token_start = next_pos;
	
	while (*next_pos != '\0' && strchr(delim, *next_pos) == NULL)
		next_pos++;
		
	if (*next_pos != '\0')
	{
		*next_pos = '\0';
		next_pos++;
	}
	
	return token_start;
}
 
size_t location_parse(FILE *ifp, char *search, Location **choices)
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
			perror("parse copy calloc");
			ERR_EXIT;
		}
		strcpy(copy, buffer);
		
		char *token = strtok_E(copy, "\t");
		int field_count = 0;
		char *fields[19] = {NULL};
		
		while (token != NULL && field_count < 19)
		{
			fields[field_count] = calloc(1, strlen(token) + 1);
			strcpy(fields[field_count], token);
			field_count++;
			token = strtok_E(NULL, "\t");
		}
		
		Location *local = NULL;
		if (field_count > 1 && strcasestr(fields[1], search) != NULL)
		{
			local = calloc(1, sizeof(Location));
			local->city = 		fields[2];
			local->state = 		fields[10];
			local->country =	fields[8];
			local->timezone = 	fields[17];
			local->latitude =	fields[4];
			local->longitude = 	fields[5];
			choices[i++] = local;
		}
		free(copy);
	}
	return i;
}

void print_menu(Location **choices, size_t n_choices)
{
	int c;
	ITEM **cities;
	MENU *city_menu;
	char buffer[256] = {0};
	cities = calloc(n_choices + 1, sizeof(ITEM *));
	if (!cities)
	{
		perror("cities calloc");
		ERR_EXIT;
	}

	char *combined_location = {NULL};

	for (size_t i = 0; i < n_choices; ++i)
	{
		combined_location = malloc(256);
		if(!combined_location)
		{
			perror("combined location malloc");
			ERR_EXIT;
		}
	
		snprintf(buffer, sizeof(buffer), "%-25.25s %.2s %.2s %-10s %-5s %s",
			choices[i]->city,
			choices[i]->state,
			choices[i]->country,
			choices[i]->timezone,
			choices[i]->latitude,
			choices[i]->longitude);
		

		strcpy(combined_location, buffer);
		cities[i] = new_item(combined_location, NULL);
	}
	cities[n_choices] = NULL;

	city_menu = new_menu((ITEM **)cities);	
	if (city_menu == NULL) 
	{
		fprintf(stderr, "ERROR: new_menu failed!");
		getch();
		endwin();
		free(combined_location);
		return;
	}
	menu_opts_off(city_menu, O_NONCYCLIC);
	
	int post_result = post_menu(city_menu);
	if (post_result != E_OK)
	{
		fprintf(stderr, "ERROR: post_menu failed!, code %d", post_result);
		getch();
		endwin();
		free(combined_location);
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
	for (size_t i = 0; i < n_choices; ++i)
	{
		free_item(cities[i]);
	}
	free(combined_location);
}
		

int main_search(char *argv)
{
	FILE *fp;
	const char *path = "cities";
	char *search = argv;
	size_t n_choices = 0;
	
	Location **choices = calloc(1, sizeof(Location *) * 100);
	if (!choices)
	{
		perror("choices calloc");
		ERR_EXIT;
	}
	
	fp = fopen(path, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "can't open %s\n", path);
		free(choices);
		ERR_EXIT;
	}
	
	initscr();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);

	n_choices = location_parse(fp, search, choices);
	
	if (n_choices >= 100)
	{
		fprintf(stderr, "too many results, be more precise\n");
		getch();
		n_choices = location_parse(fp, search, choices);
	}
	
	if (n_choices == 0)
	{
		fprintf(stderr, "no search results\n");
		getch();
		n_choices = location_parse(fp, search, choices);
	}
	
	print_menu(choices, n_choices);
	
	clear();
	refresh();
	fclose(fp);
	free(choices);
	endwin();
	return 0;
}
