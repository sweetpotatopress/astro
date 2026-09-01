/*This program is free software: you can redistribute it and/or modify
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
#include "swephexp.h"
#include "astro.h"
#include "io.h"
#include "indat.h"
#include "draw.h"

#define GEONAMEID 0
#define SNAME 1
#define ASCIINAME 2
#define SLAT 4
#define SLON 5
#define COUNTRYCODE 8
#define SSTATE 10
#define STIMEZONE 17
#define SMAX 19

/* [0]geonameid, [1]name, [2]asciiname, [3]alternatename, 
[4]latitude, [5]longitude, [6]feature class, [7]feature code,
[8]country code, [9]cc2, [10]admin1 code, [11]admin2 code,
[12]admin3 code, [13]admin4 code, [14]population, 
[15]elevation, [16]dem, [17]timezone, [18]modification date */

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

static char *xstrtok(char *s, const char *delim)
{ // doesnt skip repeating delims :3
	static char *p = NULL;
	char *start;
	
	if (s)
		p = s;
	if (!p || !*p)
		return NULL;
	if (strchr(delim, *p))
	{
		p++;
		return "E";
	}
	start = p;
	while (*p && !strchr(delim, *p))
		p++;
	if (*p)
		*p++ = 0;
	return start;
}

static char *xstrdup(const char *s)
{
	size_t l = strlen(s);
	char *d = malloc(l+1);
	if (!d)
		ERR_EXIT("ERR:xstrdup");
	return memcpy(d, s, l+1);
}

ITEM **item_range(ITEM **items, size_t item_count, size_t first, size_t last)
{
	if (first > item_count)
		first = item_count;
	if (last > item_count)
		last = item_count;
		
	size_t count = last - first;
		
	ITEM **subset = calloc(count + 1, sizeof *subset);
	if (!subset)
		ERR_EXIT("subset calloc");
		
	for (size_t i = 0; i < count; ++i)
		subset[i] = items[first + i];
	subset[count] = NULL;
	return subset;
}

static void print_menu(FIELD *cdata_field[], FORM *cdata_form, struct cdata *cdata,
struct cdata **search_result, ITEM **item_result,
int max_width, size_t search_count, size_t *city_choice)
{
	size_t per_page_max = 16;
	size_t page_count = (search_count + per_page_max - 1) / per_page_max;
	
	for(size_t i = 0; i < page_count; ++i)
	{
		size_t first_page = i * per_page_max;
		size_t remain = search_count - first_page;
		size_t page_items = remain < per_page_max ? remain : per_page_max;
		
		size_t last_page = first_page + page_items;
		
		ITEM **item_visible = item_range(item_result, search_count, first_page, last_page);
		
		int height = (int)page_items + 2;
		int width = max_width + 4;
	
		if (width > COLS)
			width = COLS - 2;
		
		int starty = (LINES - height) / 2;
		int startx = (COLS - width) / 2;
	
		MENU *city_menu = new_menu(item_visible);	
		WINDOW *city_win = newwin(height, width, starty, startx);
		WINDOW *city_subwin = derwin(city_win, height - 2, width - 2, 1, 1);
		
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
				case '\n': case 'l': case KEY_RIGHT:
					selected = current_item(city_menu);
					iret = item_index(selected) + (int)first_page;
					
					set_field_buffer(cdata_field[CITY], 0, search_result[iret]->city);
					set_field_buffer(cdata_field[TIMEZONE], 0, search_result[iret]->timezone);
					set_field_buffer(cdata_field[LATITUDE], 0, search_result[iret]->latitude);
					set_field_buffer(cdata_field[LONGITUDE], 0, search_result[iret]->longitude);
					
					memcpy(cdata->state, search_result[iret]->state, strlen(search_result[iret]->state) + 1);
					memcpy(cdata->country, search_result[iret]->country, strlen(search_result[iret]->country) + 1);
					
					werase(city_win);
					menu_done = 1;
					*city_choice = 1;
					break;
				case 'q':
					form_driver(cdata_form, REQ_CLR_FIELD);
					werase(city_win);
					menu_done = 1;
					break;
			}	
			wrefresh(city_win);
		}
		unpost_menu(city_menu);
		free_menu(city_menu);
		delwin(city_subwin);
		delwin(city_win);
		free(item_visible);
		if (*city_choice)
			break;
	}
}

static void fill_result(struct cdata **search_result, char **full_result, ITEM **item_result,
int *max_width, size_t search_count)
{
	for (size_t i = 0; i < search_count; ++i)
	{
		full_result[i] = malloc(MAXBUF);
		if(!full_result[i])
			ERR_EXIT("print_menu full_result[i] malloc");
	
		snprintf(full_result[i], MAXBUF,
		"%-25.25s %.2s %.2s %-15s %-5s %s",
			search_result[i]->city,
			search_result[i]->state,
			search_result[i]->country,
			search_result[i]->timezone,
			search_result[i]->latitude,
			search_result[i]->longitude);
			
		int len = (int)strlen(full_result[i]);
		if (len > *max_width)
			*max_width = len;
			
		item_result[i] = new_item(full_result[i], NULL);
	}
}

void city_search(FIELD *cdata_field[], FORM *cdata_form, char *search,
struct cdata *cdata, char xdg_path[])
{
	xdg_check(xdg_path, "city-db");
	
	FILE *fp = fopen(xdg_path, "r");
	if (fp == NULL)
		ERR_EXIT("city_search fopen");
	
	char *field[SMAX] = {0};
	for (int i = 0; i < SMAX; ++i)
	{
		field[i] = calloc(MAXBUF, sizeof(*field[i]));
		if (!field[i])
			ERR_EXIT("calloc");
	}
	
	int max_width = 0;
	
	size_t search_count = 0;
	size_t city_choice = 0;
	
	char buffer[MAXBUF] = {0};
	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		for (int i = 0; i < SMAX; ++i)
			field[i][0] = '\0';
		    
		size_t len = strlen(buffer);
		if (len > 0 && buffer[len - 1] == '\n')
			buffer[len - 1] = '\0';
		
		char *token = xstrtok(buffer, "\t");
		int field_count = 0;
	
		while (token != NULL && field_count < SMAX)
		{
			memcpy(field[field_count], token, strlen(token) + 1);
			field_count++;
			token = xstrtok(NULL, "\t");
		}
		
		if (field[COUNTRYCODE] != NULL && xstrcasestr(field[ASCIINAME], search) != NULL)
			++search_count;
	}
	memset(buffer, 0, MAXBUF);
	rewind(fp);
	
	struct cdata **search_result = calloc(search_count, sizeof(struct cdata *));
	if (!search_result)
		ERR_EXIT("city_search search_result calloc");
		
	char **full_result = calloc(search_count, sizeof(char *));
	if (!full_result)
		ERR_EXIT("print_menu full_result calloc");
	
	ITEM **item_result = calloc(search_count, sizeof(ITEM *));
	if (!item_result)
		ERR_EXIT("print_menu citties calloc");
	
	int count = 0;
	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		for (int i = 0; i < SMAX; ++i)
			field[i][0] = '\0';
	
		size_t len = strlen(buffer);
		if (len > 0 && buffer[len - 1] == '\n')
			buffer[len - 1] = '\0';
		
		char *token = xstrtok(buffer, "\t");
		int field_count = 0;
	
		while (token != NULL && field_count < SMAX)
		{
			memcpy(field[field_count], token, strlen(token) + 1);
			field_count++;
			token = xstrtok(NULL, "\t");
		}
		
		if (field[COUNTRYCODE] != NULL && xstrcasestr(field[ASCIINAME], search) != NULL)
		{
			search_result[count] = malloc(sizeof(*search_result[count]));
			if (!search_result[count])
				ERR_EXIT("search_result[count] malloc");
				
			search_result[count]->city = xstrdup(field[ASCIINAME]);
			search_result[count]->state = xstrdup(field[SSTATE]);
			search_result[count]->country = xstrdup(field[COUNTRYCODE]);
			search_result[count]->timezone = xstrdup(field[STIMEZONE]);
			search_result[count]->latitude = xstrdup(field[SLAT]);
			search_result[count]->longitude = xstrdup(field[SLON]);
			++count;
		}
	}
	
	fclose(fp);
	
	fill_result(search_result, full_result, item_result, &max_width, search_count);
	
	if (search_count == 0)
	{
		printw("no results");
		getch();
		goto cleanup;
	}
	  
	print_menu(cdata_field, cdata_form, cdata, search_result, item_result,
	max_width, search_count, &city_choice);
	
	cleanup:
	for (size_t i = 0; i < search_count; ++i)
	{
		free(full_result[i]);
		free_item(item_result[i]);
	    free(search_result[i]->city);
	    free(search_result[i]->state);
	    free(search_result[i]->country);
	    free(search_result[i]->timezone);
	    free(search_result[i]->latitude);
	    free(search_result[i]->longitude);
	    free(search_result[i]);
	}
	for (int i = 0; i < SMAX; ++i)
		free(field[i]);
	free(full_result);
	free(item_result);
	free(search_result);
}
