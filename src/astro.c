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

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <ncurses.h>
#include <form.h>
#include <panel.h>
#include "astro.h"
#include "draw.h"
#include "chronos.h"

#define VERSION 0.56

enum mode mode = INSERT;

void buff_trim(FIELD *current, char *buffer)
{
	char *f_buf = field_buffer(current, 0);
	int len = 0;
	field_info(current, NULL, &len, NULL, NULL, NULL, NULL);
	
	if (len <= 0)
	{
		buffer[0] = '\0';
		return;
	}
	
	memcpy(buffer, f_buf, (size_t)len);
	
	// decrement one to be in bounds, trim
	--len;
	while(len >= 0 && buffer[len] == ' ')
	{
		if (buffer[len] == '\n')
			buffer[len] = 0;
		--len;
	}
	buffer[len + 1] = '\0';
}

void field_to_member
(struct cdata *cdata, FORM *cdata_form,
FIELD *cdata_field[], char *citybuffer)
{
	FIELD *current = current_field(cdata_form);
	int index = field_index(current);
	
	char *endptr = NULL;
	long iret;
	double dret;
	errno = 0;
	
	char *buffer = malloc(MAXBUF);
	if (!buffer)
		ERR_EXIT("field_to_member buffer");
	
	buff_trim(current, buffer);
	
	switch(index)
	{
		case CITY:
			main_search(cdata_field, buffer);
			form_driver(cdata_form, REQ_VALIDATION);
			
			buff_trim(current, buffer);
			
			memcpy(citybuffer, buffer, strlen(buffer) + 1);
			
			break;
			
		case YEAR:
			iret = strtol(buffer, &endptr, 10);
			if (errno != ERANGE)
				cdata->tm_year = (int)iret;
			else
				cdata->tm_year = 1970;
			break;
			
		case MONTH:
			iret = strtol(buffer, &endptr, 10);
			if (errno != ERANGE && iret != -1)
				cdata->tm_mon = (int)iret;
			else
				cdata->tm_mon = 1;
			break;
			
		case DAY: 
			iret = strtol(buffer, &endptr, 10);
			if (errno != ERANGE && iret != -1)
				cdata->tm_mday = (int)iret;
			else
				cdata->tm_mday = 1;
			break;
			
		case HOUR:
			iret = strtol(buffer, &endptr, 10);
			if (errno != ERANGE && iret != -1) 
				cdata->tm_hour = (int)iret;
			else
				cdata->tm_hour = 1;
			break;
			
		case MINUTE:
			iret = strtol(buffer, &endptr, 10);
			if (errno != ERANGE && iret != -1)
				cdata->tm_min = (int)iret;
			else
				cdata->tm_min = 1;
			break;
			
		case TIMEZONE:
			if (setenv("TZ", buffer, 1) != 0)
				ERR_EXIT("ERR: TZ setenv fail field_to_member");
			tzset();
			break;
			
		case LATITUDE:
			dret = strtod(buffer, &endptr);
			if (errno != ERANGE)
				cdata->dlat = dret;
			else
				cdata->dlat = 0.0;
			break;
			
		case LONGITUDE:
			dret = strtod(buffer, &endptr);
			if (errno != ERANGE)
				cdata->dlon = dret;
			else
				cdata->dlon = 0.0;
			break;
	}
	free(buffer);
}

void field_label(WINDOW *cdata_form_win, int starty, int startx)
{
	const char *c_labels[] = {
		"city search:",
		"year:",
		"month:",
		"day:",
		"hour:",
		"minute:",
		"timezone:",
		"lat.",
		"long.",
		NULL
	};
	
	size_t i = CITY;
	for (starty = 4; i < FIELDMAX; ++i, starty+= 2)
			mvwprintw(cdata_form_win, starty, startx - 12,
			"%s", c_labels[i]);
	wrefresh(cdata_form_win);
}

void set_localtime(FIELD *cdata_field[])
{	// autofills chart field input with local systemtime 
	char buff[128] = {0};
	ssize_t len = readlink("/etc/localtime", buff, sizeof(buff) - 1);
	if (len != -1)
	{
		buff[len] = 0;
		
		char *tz = strstr(buff, "zoneinfo/");
		if (tz)
			memmove(buff, tz + 9, strlen(tz + 9) + 1);
	}
	setenv("TZ", buff, 1);
	tzset();
	
	struct tm *gettime = malloc(sizeof(struct tm));
	if (!gettime)
		ERR_EXIT("set_locatime() gettime malloc");
		
	time_t now = time(NULL);
	localtime_r(&now, gettime);
	set_field_buffer(cdata_field[TIMEZONE], 0, buff);
	
	memset(buff, 0, sizeof(buff));
	snprintf(buff, sizeof(buff), "%d", gettime->tm_year+1900);
	set_field_buffer(cdata_field[YEAR], 0, buff);
	
	memset(buff, 0, sizeof(buff));
	snprintf(buff, sizeof(buff), "%d", gettime->tm_mon + 1);
	set_field_buffer(cdata_field[MONTH], 0, buff);
	
	memset(buff, 0, sizeof(buff));
	snprintf(buff, sizeof(buff), "%d", gettime->tm_mday);
	set_field_buffer(cdata_field[DAY], 0, buff);
	
	memset(buff, 0, sizeof(buff));
	snprintf(buff, sizeof(buff), "%d", gettime->tm_hour);
	set_field_buffer(cdata_field[HOUR], 0, buff);
	
	memset(buff, 0, sizeof(buff));
	snprintf(buff, sizeof(buff), "%d", gettime->tm_min);
	set_field_buffer(cdata_field[MINUTE], 0, buff);
	
	free(gettime);
}

void validate_fields(FIELD *cdata_field[],
FORM *cdata_form, struct cdata *cdata, char *citybuffer)
{
	size_t i = 0;
	
	// save city name
	set_current_field(cdata_form, cdata_field[i]);
	FIELD *current = current_field(cdata_form);
	char buffer[MAXBUF] = {0};
	buff_trim(current, buffer);
	memcpy(citybuffer, buffer, strlen(buffer) + 1);
	++i;
		
	for (; i < FIELDMAX; i++)
	{
		set_current_field(cdata_form, cdata_field[i]);
		form_driver(cdata_form, REQ_VALIDATION);
		field_to_member(cdata, cdata_form,
		cdata_field, citybuffer);
	}
}
	
void input_chart_data(struct io *io, struct cdata *cdata, char *citybuffer)
{
	WINDOW *cdata_form_win;
	FIELD *cdata_field[10];
	FORM *cdata_form;
	int ch;
	int starty, startx;
	size_t i = 0;
	
	cbreak();
	noecho();
	curs_set(1);
	
	cdata_form_win = newwin(LINES, COLS, 0, 0);
	
	keypad(cdata_form_win, TRUE);	
	
	wbkgdset(cdata_form_win, COLOR_PAIR(M_COLOR));
	
	starty = 4;
	startx = 18;
	
	cdata_field[CITY] = new_field(1, 25, starty, startx, 0, 0);
	set_field_back(cdata_field[CITY], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(cdata_field[CITY], O_STATIC);
	field_opts_off(cdata_field[CITY], O_AUTOSKIP);
	starty += 2;
	
	cdata_field[YEAR] = new_field(1, 6, starty, startx, 0, 0);
	set_field_back(cdata_field[YEAR], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(cdata_field[YEAR], O_AUTOSKIP);
	starty += 2;
	
	cdata_field[MONTH] = new_field(1, 3, starty, startx, 0, 0);
	set_field_back(cdata_field[MONTH], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(cdata_field[MONTH], O_AUTOSKIP);
	starty += 2;
	
	cdata_field[DAY] = new_field(1, 3, starty, startx, 0, 0);
	set_field_back(cdata_field[DAY], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(cdata_field[DAY], O_AUTOSKIP);
	starty += 2;
	
	cdata_field[HOUR] = new_field(1, 3, starty, startx, 0, 0);
	set_field_back(cdata_field[HOUR], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(cdata_field[HOUR], O_AUTOSKIP);
	starty+= 2;
	
	cdata_field[MINUTE] = new_field(1, 3, starty, startx, 0, 0);
	set_field_back(cdata_field[MINUTE], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(cdata_field[MINUTE], O_AUTOSKIP);
	starty+= 2;
	
	cdata_field[TIMEZONE] = new_field(1, 30, starty, startx, 0, 0);
	set_field_back(cdata_field[TIMEZONE], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(cdata_field[TIMEZONE], O_STATIC);
	field_opts_off(cdata_field[TIMEZONE], O_AUTOSKIP);
	starty += 2;
	
	cdata_field[LATITUDE] = new_field(1, 11, starty, startx, 0, 0);
	set_field_back(cdata_field[LATITUDE], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(cdata_field[LATITUDE], O_AUTOSKIP);
	starty+= 2;
	
	cdata_field[LONGITUDE] = new_field(1, 11, starty, startx, 0, 0);
	set_field_back(cdata_field[LONGITUDE], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(cdata_field[LONGITUDE], O_AUTOSKIP);
	
	cdata_field[FIELDMAX] = NULL;

	cdata_form = new_form(cdata_field);
	set_form_win(cdata_form, cdata_form_win);
	set_form_sub(cdata_form,
	derwin(cdata_form_win, LINES, COLS, 0, 0));
	
	post_form(cdata_form);
	
	set_current_field(cdata_form, cdata_field[CITY]);
	
	field_label(cdata_form_win, starty, startx);
	pos_form_cursor(cdata_form);
	
	int cdata_entry = 0;
	while(!cdata_entry && (ch = wgetch(cdata_form_win)))
	{
		switch(mode)
		{	
			case NORMAL:
				switch (ch)
				{
					case 27:
						break;
						
					case 'i':
						mode = INSERT;
						break;
						
					case 'j': case KEY_DOWN:
						form_driver(cdata_form, REQ_NEXT_FIELD);
						form_driver(cdata_form, REQ_END_LINE);
						break;
						
					case 'k': case KEY_UP:
						form_driver(cdata_form, REQ_PREV_FIELD);
						form_driver(cdata_form, REQ_END_LINE);
						break;
						
					case 'h': case KEY_LEFT:
						form_driver(cdata_form, REQ_LEFT_CHAR);
						break;
						
					case 'l': case KEY_RIGHT:
						form_driver(cdata_form, REQ_RIGHT_CHAR);
						break;
						
					case 9: // tab
						set_localtime(cdata_field);
						break;
						
					case 'w':
						validate_fields(cdata_field,
						cdata_form, cdata, citybuffer);
						main_io(io, cdata, cdata_field, citybuffer, 'w');
						mode = NORMAL;
						break;
						
					case 'e':
						main_io(io, cdata, cdata_field, citybuffer, 'e');
						mode = NORMAL;
						break;
						
					case '\n':
						cdata_entry = 1;
						break;
				}
				break;
				
			case INSERT:
				switch (ch)
				{
					 case '\n':
						form_driver(cdata_form, REQ_VALIDATION);
						field_to_member(cdata, cdata_form,
						cdata_field, citybuffer);
						form_driver(cdata_form, REQ_NEXT_FIELD);
						
						field_label(cdata_form_win, starty, startx);
						
						form_driver(cdata_form, REQ_END_LINE);
						break;
						
					case KEY_DOWN:
						form_driver(cdata_form, REQ_NEXT_FIELD);
						form_driver(cdata_form, REQ_END_LINE);
						break;
						
					case KEY_UP:
						form_driver(cdata_form, REQ_PREV_FIELD);
						form_driver(cdata_form, REQ_END_LINE);
						break;
						
					case KEY_LEFT:
						form_driver(cdata_form, REQ_LEFT_CHAR);
						break;
						
					case KEY_RIGHT:
						form_driver(cdata_form, REQ_RIGHT_CHAR);
						break;
						
					case KEY_BACKSPACE:
						form_driver(cdata_form, REQ_DEL_PREV);
						break;
						
					case 27: // esc
						mode = NORMAL;
						break;
						
					default:
						form_driver(cdata_form, ch);
						break;
						
					}
					break;
		}
		wrefresh(cdata_form_win);
	}
	
	validate_fields(cdata_field,
	cdata_form, cdata, citybuffer);

	unpost_form(cdata_form);
	werase(cdata_form_win);
	wrefresh(cdata_form_win);
	free_form(cdata_form);
	
	for (i = CITY; i < FIELDMAX; ++i)
	{
		free_field(cdata_field[i]);
	}
	delwin(cdata_form_win);
}

int main()
{
	struct cdata *cdata = calloc(1, sizeof(*cdata));
	if (!cdata)
		ERR_EXIT("main Location calloc");
	cdata->city = malloc(MAXBUF);
	if (!cdata->city)
		ERR_EXIT("ERR: main cdata->city malloc");
		
	char *citybuffer = malloc(MAXBUF);
	if (!citybuffer)
		ERR_EXIT("ERR: main citybuffer alloc fail");
	
	struct pxx *pxx = calloc(1, sizeof(*pxx));
	if (!pxx)
		ERR_EXIT("main pxx");
	
	ALLOC_PLANET(dsun);
	ALLOC_PLANET(dmoon);
	ALLOC_PLANET(dmerc);
	ALLOC_PLANET(dven);
	ALLOC_PLANET(dmars);
	ALLOC_PLANET(djup);
	ALLOC_PLANET(dsat);
	ALLOC_PLANET(dura);
	ALLOC_PLANET(dnep);
	ALLOC_PLANET(dplu);
	ALLOC_PLANET(dmnod);
	ALLOC_PLANET(dtnod);
	ALLOC_PLANET(dasc);
	ALLOC_PLANET(dmc);
	ALLOC_PLANET(ddsc);
	ALLOC_PLANET(dic);
	
	struct io *io = calloc(1, sizeof(*io));
	if (!io)
		ERR_EXIT("mai io calloc");
	io->filepath = malloc(MAXBUF);
	if (!io->filepath)
		ERR_EXIT("main io->filepath malloc");
	io->filename = malloc(256);
	if (!io->filename)
		ERR_EXIT("main io->filename malloc");
		
	double cusps[13];
	
	const char *home_dir = getenv("HOME");
	if (!home_dir)
		ERR_EXIT("HOME environment not set");
		
	char fn_buff[MAXBUF] = {0};
	snprintf(fn_buff, MAXBUF, 
	"%s/.local/share/astro/ephe", home_dir);
	
	swe_set_ephe_path(fn_buff);
	
	initscr();
	set_escdelay(25);
	
	start_color();
	init_color(1, 0, 0, 0); //black
	init_color(2, 800, 800, 1000); //white
	init_color(3, 1000, 600, 600); //red
	init_color(4, 600, 1000, 600); //green
	init_color(5, 1000, 1000, 600); //yellow
	init_color(6, 500, 500, 1000); //blue
	
	init_pair(M_COLOR, 2, 1);
	init_pair(FIRE, 3, 1);
	init_pair(EARTH, 4, 1);
	init_pair(AIR, 5, 1);
	init_pair(WATER, 6, 1);
	
	WINDOW *main_win;
	main_win = newwin(LINES, COLS, 0, 0);
	
	PANEL *planet_panel;
	WINDOW *planet_win = newwin(PWINY, PWINX, PWIN_Y, PWIN_X);
	planet_panel = new_panel(planet_win);
	hide_panel(planet_panel);
	
	wbkgdset(planet_win, COLOR_PAIR(M_COLOR));
	
	PANEL *retro_panel;
	WINDOW *retro_win = newwin(RWINY, RWINX, RWIN_Y, RWIN_X);
	retro_panel = new_panel(retro_win);
	hide_panel(retro_panel);
	
	wbkgdset(retro_win, COLOR_PAIR(M_COLOR));
	
	keypad(main_win, TRUE);
	keypad(stdscr, TRUE);

	wbkgdset(main_win, COLOR_PAIR(M_COLOR));
	werase(stdscr);
	wrefresh(stdscr);
	
	int main_done = 0;
	while (!main_done)
	{
		input_chart_data(io, cdata, citybuffer);
		cdata->city = citybuffer;
		pxx_fill(cusps, cdata, pxx);
		draw_chart(main_win, cusps, pxx);
		cur_chart_data(main_win, io, cdata);
			
		static int retro_trig = 0, planet_trig = 0;
		int chart_done = 0, ch = 0;
		while(!chart_done && !main_done &&
		(ch = wgetch(main_win)))
		{
			switch(ch)
			{
				case '\n':
					animate_chart(main_win, planet_win, retro_win,
					&planet_panel, &retro_panel,
					io, cdata, pxx, 
					&planet_trig, &retro_trig, cusps);
					doupdate();
					break;
			case 9: // tab
					realtime_chart(main_win, planet_win, retro_win,
					&planet_panel, &retro_panel,
					io, cdata, pxx,
					&planet_trig, &retro_trig, cusps);
					doupdate();
					break;
				case 'q':
					main_done = 1;
					chart_done = 1;
					break;
				case 'i':
					werase(main_win);
					wrefresh(main_win);
					free(io->filename);
					io->filename = NULL;
					planet_trig = 0;
					retro_trig = 0;
					chart_done = 1;
					mode = INSERT;
					break;
				case 'p':
					if (!planet_trig)
					{
						planet_table(planet_win, pxx);
						show_panel(planet_panel);
						planet_trig = 1;
					}
					else
					{
						hide_panel(planet_panel);
						clear();
						refresh();
						planet_trig = 0;
					}
					
					if (retro_trig > 0)
					{
						retrograde_table(retro_win, pxx);
						show_panel(retro_panel);
					}
					
					touchwin(main_win);
					wnoutrefresh(main_win);
					update_panels();
					doupdate();
					break;
				case 'o':
					if (!retro_trig)
					{
						retrograde_table(retro_win, pxx);
						show_panel(retro_panel);
						retro_trig = 1;
					}
					else
					{
						hide_panel(retro_panel);
						clear();
						refresh();
						retro_trig = 0;
					}	
					
					if (planet_trig > 0)
					{
						planet_table(planet_win, pxx);
						show_panel(planet_panel);
					}
					
					touchwin(main_win);
					wnoutrefresh(main_win);
					update_panels();
					doupdate();
					break;
				default:
					break;
			}
		}
	}
	delwin(main_win);
	endwin();
	swe_close();
	
	free(cdata);
	free(citybuffer);
	free(pxx);
	
	return 0;
} 
