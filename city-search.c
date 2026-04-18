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
 
int location_search_parse(FILE *ifp, char *search, Location **choices, int max_choices)
{
	int i = 0;
	char line[1024] = {0};
	
	while (fgets(line, sizeof(line), ifp) != NULL)
	{
		int len = strlen(line);
		if (line[len - 1] == '\n')
			line[len - 1] = '\0';
		
		char *copy = malloc(strlen(line) + 1);
		strcpy(copy, line);
		
		char *token = strtok(copy, "\t");
		int field_count = 0;
		char *fields[19] = {0};
				
		while (token != NULL && field_count < 19)
		{
			fields[field_count] = malloc(strlen(token) + 1);
			strcpy(fields[field_count], token);
			field_count++;
			token = strtok(NULL, "\t");
		}
		
		if (field_count > 1 && strcasestr(fields[1], search) != NULL)
		{
			Location *location = malloc(sizeof(Location));
			location->city = fields[1];
			location->country = fields[8];
			location->latitude = fields[4];
			location->longitude = fields[5];
			choices[i++] = location;
		}
		else
		{
			for(int j = 0; j < field_count; j++)
				free(fields[j]);
		}
		free(copy);
	}
	if (i >= max_choices)
		return -1;
	else
		return i;
}

void print_menu(Location **choices)
{
	int c;
	ITEM **cities;
	MENU *city_menu;
	char buffer[256];
	cities = (ITEM **)calloc(n_choices + 1, sizeof(ITEM *));

	for (int i = 0; i < n_choices; ++i)
	{
		snprintf(buffer, sizeof(buffer), "%-30s %-20s %s %s",
			choices[i]->city,
			choices[i]->country,
			choices[i]->latitude,
			choices[i]->longitude);
		
		char *item_name = malloc(strlen(buffer) + 1);
		strcpy(item_name, buffer);
		cities[i] = new_item(item_name, NULL);
	}
	cities[n_choices] = NULL;
	
	city_menu = new_menu((ITEM **)cities);	
	if (city_menu == NULL) 
	{
		mvprintw(LINES - 3, 0, "ERROR: new_menu failed!");
		refresh();
		getch();
		return;
	}
	menu_opts_off(city_menu, O_ONEVALUE);
	set_menu_format(city_menu, LINES - 5, 1);
	
	int post_result = post_menu(city_menu);
	if (post_result != E_OK)
	{
		mvprintw(LINES - 3, 0, "ERROR: post_menu failed!, code %d", post_result);
		refresh();
		getch();
		return;
	}
	
	mvprintw(LINES - 2, 0, "hehe");
	refresh();

	while((c = getch()) != KEY_F(1))
	{
		switch(c)
		{
			case 'k':
				menu_driver(city_menu, REQ_DOWN_ITEM);
				break;
			case 'j':
				menu_driver(city_menu, REQ_UP_ITEM);
				break;
		}
	}
	
	unpost_menu(city_menu);
	for (int i = 0; i < n_choices; ++i)
		free_item(cities[i]);
	free_menu(city_menu);
	free(cities);
}
		

int main_search(char *argv)
{
	FILE *fp;
	const char *path = "cities";
	char *search = argv;
	int max_loc = 100;
	Location **choices = malloc(sizeof(Location *) * max_loc);
	
	
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

	n_choices = location_search_parse(fp, search, choices, max_loc);
	
	if (n_choices == -1 || n_choices >= 100)
	{
		fprintf(stderr, "too many results, be more precise\n");
		getch();
		endwin();
		goto exit_err;
	}
	
	fclose(fp);
	if (n_choices == 0)
	{
		fprintf(stderr, "no search results\n");
		getch();
		endwin();
		goto exit_err;
	}
	
	if (ferror(stdout)) 
	{
		fprintf(stderr, "error writing stdout\n");
		getch();
		endwin();
		goto exit_err;
	}
	
	print_menu(choices);
	
	exit_err:
	clear();
	refresh();
	for (int i = 0; i < n_choices; ++i)
	{
		if (choices[i] != NULL)
		{
			free(choices[i]->city);
			free(choices[i]->country);
			free(choices[i]->latitude);
			free(choices[i]->longitude);
			free(choices[i]);
		}
	}
	endwin();
	return 0;
}
