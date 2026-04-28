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
 
size_t location_parse(FILE *ifp, char *search, Location **choices)
{
	size_t i = 0;
	char buffer[1024] = {0};
	char *tokens[19] = {NULL};
	size_t token_idx = 0;
	size_t start = 0;
	size_t choice_count = 0;
	
	while (fgets(buffer, sizeof(buffer), ifp) != NULL)
	{
		char *copy = calloc(1, strlen(buffer) + 1);
			if (!copy)
			{
				perror("parse copy calloc");
				ERR_EXIT;
			}
		strcpy(copy, buffer);
		
		char *buf = NULL;	
		for (i = 0; buffer[i] != '\n' && buffer[i] != '\0'; ++i)
		{
			
			size_t token_len = i - start + (buffer[i] != '\t' ? 1 : 0);
			buf = malloc(token_len + 1);
			if (!buf)
			{
				perror("parse buf malloc");
				ERR_EXIT;
			}
			
			memcpy(buf, copy + start, token_len);
			buf[token_len] = '\0';
			tokens[token_idx++] = buf;
			start = i + 1;
		}
		
		Location *local =  NULL;
		if (token_idx > 1 && strcasestr(tokens[0], search) != NULL)
		{
			local = calloc(1, sizeof(Location));
			if (!local)
			{
				perror("parse local calloc");
				ERR_EXIT;
			}
			local->city = strdup(tokens[2]);
			local->state = strdup(tokens[9]);
			local->country = strdup(tokens[8]);
			local->timezone = strdup(tokens[17]);
			local->latitude = strdup(tokens[4]);
			local->longitude = strdup(tokens[5]);
			choices[choice_count++] = local;
			printw("test: %s", local->city);
		}
		else
			free(local);
			
		for (size_t j = 0; j < token_idx; ++j)
			free(tokens[j]);
		
		free(copy);
		token_idx = 0;
		start = 0;
	}
	return choice_count;
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
	
		snprintf(buffer, sizeof(buffer), "%-25.25s %.2s %-10s %-5s %-5s %s",
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
		free((char *)item_name(cities[i]));
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
	for (size_t i = 0; i < n_choices; ++i)
		free(choices[i]);
	endwin();
	return 0;
}
