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

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <menu.h>

char* strtok_E(char *str, const char *delim)
{
	//strtok that doesnt skip repeating delim's.
	//correctly parses geonames data :3
	
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
 
size_t location_parse(FILE *ifp, char *search,
Location ***choices, size_t *max_search)
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
			endwin();
			perror("parse copy calloc");
			ERR_EXIT;
		}
		memcpy(copy, buffer, strlen(buffer) + 1);
		
		char *token = strtok_E(copy, "\t");
		int field_count = 0;
		char *fields[19] = {NULL};
		
		while (token != NULL && field_count < 19)
		{
			fields[field_count] = calloc(1, strlen(token) + 1);
			memcpy(fields[field_count], token, strlen(token) + 1);
			field_count++;
			token = strtok_E(NULL, "\t");
		}
		
		Location *local = NULL;
		if (field_count > 1 && strcasestr(fields[1], search) != NULL)
		{
			if (i >= *max_search)
			{
				*max_search *= 2;
				Location **temp = reallocarray(
				*choices, *max_search, sizeof(Location*));
				if (!temp)
				{
					endwin();
					perror("choices realloc");
					ERR_EXIT;
				}
				*choices = temp;
			}
			
			local = calloc(1, sizeof(Location));
			if (!local)
			{
				endwin();
				perror("local parser");
				ERR_EXIT;
			}
			local->city = 		fields[2];	fields[2] = NULL;
			local->state = 		fields[10];	fields[10] = NULL;
			local->country =	fields[8];	fields[8] = NULL;
			local->timezone = 	fields[17]; fields[17] = NULL;
			local->latitude =	fields[4]; 	fields[4] = NULL;
			local->longitude = 	fields[5];	fields[5] = NULL;
			(*choices)[i++] = local;
		}
		free(copy);
		for (int j = 0; j < field_count; j++)
			free(fields[j]);
	}
	return i;
}

void print_menu(FIELD *cdata_field[], Location **choices, size_t n_choices)
{
	int ch;
	ITEM **cities;
	MENU *city_menu;
	WINDOW *city_win;
	WINDOW *city_subwin;
	//use ** to not lose the pointer after the loop
	char **strings = calloc(n_choices, sizeof(char *));
	char buffer[1024] = {0};
	int max_width = 0;
	
	cities = calloc(n_choices + 1, sizeof(ITEM *));
	if (!cities)
	{
		endwin();
		perror("cities calloc");
		ERR_EXIT;
	}

	for (size_t i = 0; i < n_choices; ++i)
	{
		strings[i] = malloc(sizeof(buffer));
		if(!strings[i])
		{
			endwin();
			perror("strings[i] malloc");
			ERR_EXIT;
		}
	
		snprintf(buffer, sizeof(buffer),
		"%-25.25s %.2s %.2s %-15s %-5s %s",
			choices[i]->city,
			choices[i]->state,
			choices[i]->country,
			choices[i]->timezone,
			choices[i]->latitude,
			choices[i]->longitude);
			
		memcpy(strings[i], buffer, strlen(buffer) + 1);
		
		int len = (int)strlen(buffer) + 1;
		if (len > max_width)
			max_width = len;

		cities[i] = new_item(strings[i], NULL);
		set_item_userptr(cities[i], (void *)choices[i]);
	}
	cities[n_choices] = NULL;
	
	city_menu = new_menu((ITEM **)cities);	
		if (!city_menu) 
		{
			endwin();
			perror("city_menu");
			getch();
		}
	
	int width = max_width + 4;
	int height = (int)n_choices + 3;
	
	if (width > COLS)
		width = COLS - 2;
	if (height > LINES)
		height = 18;
		
	int starty = (LINES - height) / 2;
	int startx = (COLS - width) / 2;
	
	city_win = newwin(height, width, starty, startx);
	if (!city_win)
	{
		endwin();
		fprintf(stderr, "ERR: city_win failed");
		getch();
	}
	keypad(city_win, TRUE);
	clearok(city_win, TRUE);
	wclear(city_win);
	wrefresh(city_win);
	
	box(city_win, 0, 0);
	
	city_subwin = derwin(city_win, height - 2, width - 2, 1, 1);
	
	set_menu_win(city_menu, city_win);
	set_menu_sub(city_menu, city_subwin);
	menu_opts_off(city_menu, O_NONCYCLIC);
	
	int iret = post_menu(city_menu);
	if (iret != E_OK)
	{
		endwin();
		fprintf(stderr, "ERR: post_menu failed!, %d", iret);
		getch();
	}
	
	int menu_done = 0;
	while(!menu_done && (ch = GET_INPUT(city_win)))
	{
		switch(ch)
		{
			case 'j': case KEY_DOWN:
				menu_driver(city_menu, REQ_DOWN_ITEM);
				break;
			case 'k': case KEY_UP:
				menu_driver(city_menu, REQ_UP_ITEM);
				break;
			case '\n':
				ITEM *selected = current_item(city_menu);
				Location *cdata = (Location *)item_userptr(selected);
				
				set_field_buffer(cdata_field[0], 0, cdata->city);
				set_field_buffer(cdata_field[6], 0, cdata->timezone);
				set_field_buffer(cdata_field[7], 0, cdata->latitude);
				set_field_buffer(cdata_field[8], 0, cdata->longitude);
				
				menu_done = 1;
				break;
			default:
				break;
		}	
		wrefresh(city_win);
	}
	
	unpost_menu(city_menu);
	touchwin(city_win);
	wrefresh(city_win);
	free_menu(city_menu); //free menu first
	for (size_t i = 0; i < n_choices; ++i)
	{
		free_item(cities[i]);
		free(strings[i]);
	}
	free(strings);
	//always delwin subwin first
	delwin(city_subwin);
	delwin(city_win);
}
		

int main_search(FIELD *cdata_field[], char *argv)
{
	FILE *fp;
	const char *path = "city-db";
	char *search = argv;
	size_t n_choices = 0;
	size_t max_search = 100;
	
	Location **choices = calloc(max_search, sizeof(Location *));
	if (!choices)
	{
		endwin();
		perror("choices calloc");
		ERR_EXIT;
	}
	
	fp = fopen(path, "r");
	if (fp == NULL)
	{
		endwin();
		printw("can't open %s\n", path);
		ERR_EXIT;
	}
	
	noecho();
	cbreak();

	/* sending the address of choices allows the memory assigned by calloc to
	be realloced, hence ***Location in the function */
	n_choices = location_parse(fp, search, &choices, &max_search);

	if (n_choices == 0)
	{
		printw("no search results\n");
		getch();
		clear();
		refresh();
		fclose(fp);
		endwin();
		for (size_t j = 0; j < max_search; ++j)
			free(choices[j]);
		free(choices);
		return 0;
	}
	
	print_menu(cdata_field, choices, n_choices);
	
	clear();
	refresh();
	fclose(fp);
	endwin();
	for (size_t j = 0; j < n_choices; ++j)
		free(choices[j]);
	free(choices);
	return 0;
}
