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
#include <form.h>
#include <menu.h>
#include "astro.h"
#include "io.h"
#include "cdata.h"
#include "draw.h"

static char *xstrcasestr(const char *h, const char *n)
{
	size_t i, nl = strlen(n);
	if (!nl)
		return (char *)h;
	for (; *h; h++)
	{
		for (i = 0; i < nl; i++)
			if (tolower((unsigned char)h[i]) != tolower((unsigned char)n[i]))
				break;
		if (i == nl)
			return (char *)h;
	}
	return NULL;
}

char* strtok_E(char *str, const char *delim)
{ //strtok that doesnt skip repeating delims :3
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

void print_menu(FIELD *cdata_field[], FORM *cdata_form,
struct cdata **search_result, size_t search_count,
char *statebuffer, char *countrybuffer)
{
	MENU *city_menu;
	WINDOW *city_win;
	WINDOW *city_subwin;
	
	char **full_result = calloc(search_count, sizeof(char *));
	if (!full_result)
		ERR_EXIT("print_menu full_result calloc");

	ITEM **result_item = calloc(search_count + 1, sizeof(ITEM *));
	if (!result_item)
		ERR_EXIT("print_menu citties calloc");

	int max_width = 0;
	
	for (size_t i = 0; i < search_count; ++i)
	{
		char buffer[MAXBUF] = {0};
		full_result[i] = malloc(sizeof(buffer));
		if(!full_result[i])
			ERR_EXIT("print_menu full_result[i] malloc");
	
		snprintf(buffer, sizeof(buffer),
		"%-25.25s %.2s %.2s %-15s %-5s %s",
			search_result[i]->city,
			search_result[i]->state,
			search_result[i]->country,
			search_result[i]->timezone,
			search_result[i]->latitude,
			search_result[i]->longitude);
			
		memcpy(full_result[i], buffer, strlen(buffer) + 1);
		
		int len = (int)strlen(buffer) + 1;
		if (len > max_width)
			max_width = len;

		result_item[i] = new_item(full_result[i], NULL);
	}
	result_item[search_count] = NULL;
	
	city_menu = new_menu(result_item);	
		if (!city_menu) 
			ERR_EXIT("search city_menu new_menu");
	
	int width = max_width + 4;
	int height = (int)search_count + 2;
	
	if (width > COLS)
		width = COLS - 2;
	if (height > 18)
		height = 18;
		
	int starty = (LINES - height) / 2;
	int startx = (COLS - width) / 2;
	
	city_win = newwin(height, width, starty, startx);
	city_subwin = derwin(city_win, height - 2, width - 2, 1, 1);
		
	wbkgdset(city_win, COLOR_PAIR(M_COLOR));
		
	keypad(city_win, TRUE);
	
	box(city_win, 0, 0);
	
	set_menu_win(city_menu, city_win);
	set_menu_sub(city_menu, city_subwin);
	set_menu_fore(city_menu, COLOR_PAIR(M_COLOR) | A_REVERSE);
	set_menu_back(city_menu, COLOR_PAIR(M_COLOR));
	menu_opts_off(city_menu, O_NONCYCLIC);
	
	int iret = post_menu(city_menu);
	if (iret != E_OK)
		ERR_EXIT("search post_menu(city_menu)");
	
	ITEM *selected = NULL;
	
	int ch, menu_done = 0;
	while(!menu_done && (ch = wgetch(city_win)))
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
				selected = current_item(city_menu);
				iret = item_index(selected);
				
				set_field_buffer(cdata_field[CITY], 0,
				search_result[iret]->city);
				set_field_buffer(cdata_field[TIMEZONE], 0,
				search_result[iret]->timezone);
				set_field_buffer(cdata_field[LATITUDE], 0,
				search_result[iret]->latitude);
				set_field_buffer(cdata_field[LONGITUDE], 0,
				search_result[iret]->longitude);
				
				memcpy(statebuffer, search_result[iret]->state,
				strlen(search_result[iret]->state) + 1);
				memcpy(countrybuffer, search_result[iret]->country,
				strlen(search_result[iret]->country) + 1);
				
				werase(city_win);
				wrefresh(city_win);
				
				menu_done = 1;
				break;
			case 'q':
				form_driver(cdata_form, REQ_CLR_FIELD);
				werase(city_win);
				wrefresh(city_win);
				menu_done = 1;
				break;
			default:
				break;
		}	
		wrefresh(city_win);
	}
	
	unpost_menu(city_menu);
	free_menu(city_menu);
	for (size_t j = 0; j < search_count; ++j)
	{
		free_item(result_item[j]);
		free(full_result[j]);
	    free(search_result[j]);
	}
	free(full_result);
	free(search_result);
	delwin(city_subwin);
	delwin(city_win);
}
	
void city_search(FIELD *cdata_field[], FORM *cdata_form, char *search,
char *statebuffer, char *countrybuffer)
{
	struct cdata **search_result = calloc(MAXBUF, sizeof(struct cdata *));
	if (!search_result)
		ERR_EXIT("city_search search_result calloc");
	
	const char *home_dir = getenv("HOME");
	if (!home_dir)
		ERR_EXIT("HOME environment not set");
		
	char fn_buff[MAXBUF] = {0};
		
	const char *xdg_data = getenv("XDG_DATA_HOME");
	if (!xdg_data)
		snprintf(fn_buff, MAXBUF, 
		"%s/.local/share/astro/city-db", home_dir);
	else
		snprintf(fn_buff, MAXBUF, 
		"%s/astro/city-db", xdg_data);
	
	FILE *fp = fopen(fn_buff, "r");
	if (fp == NULL)
		ERR_EXIT("city_search fopen");
	
	size_t search_max = MAXBUF;
	size_t i = 0;
	char buffer[MAXBUF] = {0};
	
	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		size_t len = strlen(buffer);
		if (buffer[len - 1] == '\n')
			buffer[len - 1] = '\0';
		
		char *token = strtok_E(buffer, "\t");
		int field_count = 0;
		char *field[19] = {NULL};
		/* 	
			[0]geonameid, [1]name, [2]asciiname, [3]alternatename, 
			[4]latitude, [5]longitude, [6]feature class, [7]feature code,
			[8]country code, [9]cc2, [10]admin1 code, [11]admin2 code,
			[12]admin3 code, [13]admin4 code, [14]population, 
			[15]elevation, [16]dem, [17]timezone, [18]modification date
		*/
		while (token != NULL && field_count < 19)
		{
			field[field_count] = calloc(1, strlen(token) + 1);
			if (!field[field_count])
				ERR_EXIT("location_parse fields[field_count] calloc");
			
			memcpy(field[field_count], token, strlen(token) + 1);
			field_count++;
			token = strtok_E(NULL, "\t");
		}
		
		if (field_count > 1 &&
		field[1] != NULL && field[8] != NULL &&
		xstrcasestr(field[1], search) != NULL)
		{
			if (i >= search_max)
			{
				while(i >= search_max)
					search_max *= 2;
				struct cdata **temp = reallocarray(
				search_result, search_max, sizeof(struct cdata *));
				if (!temp)
					ERR_EXIT("location_parse temp realloc");
					
				search_result = temp;
			}
			
			search_result[i] = malloc(sizeof(struct cdata));
			if (!search_result[i])
				ERR_EXIT("search_result[i] malloc");
			
			search_result[i]->city = 		field[2];	field[2] = NULL;
			search_result[i]->state = 		field[10];	field[10] = NULL;
			search_result[i]->country =		field[8];	field[8] = NULL;
			search_result[i]->timezone = 	field[17]; field[17] = NULL;
			search_result[i]->latitude =	field[4]; 	field[4] = NULL;
			search_result[i]->longitude = 	field[5];	field[5] = NULL;
			++i;
		}
		for (int j = 0; j < field_count; j++)
			free(field[j]);
	}
	fclose(fp);
	
	if (i == 0)
	{
		printw("no search results\n");
		getch();
		for (size_t j = 0; j < search_max; ++j)
			free(search_result[j]);
		free(search_result);
		return;
	}
	print_menu(cdata_field, cdata_form, search_result, i,
	statebuffer, countrybuffer);
}
