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
#include <pwd.h>
#include <errno.h>
#include <swephexp.h>
#include <ncurses.h>
#include <form.h>
#include <panel.h>
#include "astro.h"

#define VERSION 0.56

Mode mode = INSERT;

// sun, moon, mercury, venus, mars, jupiter,
// saturn, uranus, neptune, pluto, mean node, true node
const char *pl_sym[] = {"(o)", "(()", "(-o<)",
"(~:o)", "(o->)", "(\\+)", "(h)", "(\\*/)", "(?)",
"(P)", NULL, "(^)"};

// 0 = NULL because the swiss ephemeris skips 0
const char *zo_sym[] = {NULL, "ari", "tau", "gem", "can",
"leo", "vir", "lib", "sco", "sag",
"cap", "aqu", "pis"};
	
int iflag, ipl;
double xx[6];	 // longitude, latitude, distance 
				// speed in long. speed in lat, speed in dist.
char serr[AS_MAXCH];
double cusps[13], ascmc[10]; //houses, asc, mc
int ihsy = 'W'; // house system


int months(int month, int year)
{
	int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	if (month == 2)
		if ((year % 4 == 0 && year % 100 != 0) || 
		(year % 400 == 0))
			return 29;
	return days[month];
}

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
(Cdata *cdata, FORM *cdata_form,
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
FORM *cdata_form, Cdata *cdata, char *citybuffer)
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
	
void input_chart_data(Io *io, Cdata *cdata, char *citybuffer)
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
						main_io(io, cdata_field, cdata, citybuffer, 'w');
						mode = NORMAL;
						break;
						
					case 'e':
						main_io(io, cdata_field, cdata, citybuffer, 'e');
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
	wclear(cdata_form_win);
	wrefresh(cdata_form_win);
	free_form(cdata_form);
	
	for (i = CITY; i < FIELDMAX; ++i)
	{
		free_field(cdata_field[i]);
	}
	delwin(cdata_form_win);
}

void element_color(WINDOW *win, int y, int x, int count,
int sign, Pxx *pxx, char ch)
{
	double *p_arr[] = {
			pxx->dsun, pxx->dmoon,
			pxx->dmerc, pxx->dven,
			pxx->dmars, pxx->djup,
			pxx->dsat, pxx->dura,
			pxx->dnep, pxx->dplu,
			pxx->dmnod, pxx->dtnod};
			
	static int j;
	
	if (strcmp("ari", zo_sym[sign]) == 0)
		j = FIRE;
	else if (strcmp("tau", zo_sym[sign]) == 0)
		j = EARTH;
	else if (strcmp("gem", zo_sym[sign]) == 0)
		j = AIR;
	else if (strcmp("can", zo_sym[sign]) == 0)
		j = WATER;
	else if (strcmp("leo", zo_sym[sign]) == 0)
		j = FIRE;
	else if (strcmp("vir", zo_sym[sign]) == 0)
		j = EARTH;
	else if (strcmp("lib", zo_sym[sign]) == 0)
		j = AIR;
	else if (strcmp("sco", zo_sym[sign]) == 0)
		j = WATER;
	else if (strcmp("sag", zo_sym[sign]) == 0)
		j = FIRE;
	else if (strcmp("cap", zo_sym[sign]) == 0)
		j = EARTH;
	else if (strcmp("aqu", zo_sym[sign]) == 0)
		j = AIR;
	else if (strcmp("pis", zo_sym[sign]) == 0)
		j = WATER;

	switch(ch)
	{
		case 'z':
			switch(j)
			{
				case FIRE:
					wattron(win, COLOR_PAIR(FIRE));
					mvwaddstr(win, y, x, zo_sym[sign]);
					wattroff(win, COLOR_PAIR(FIRE));
					break;
				case EARTH:
					wattron(win, COLOR_PAIR(EARTH));
					mvwaddstr(win, y, x, zo_sym[sign]);
					wattroff(win, COLOR_PAIR(EARTH));
					break;
				case AIR:
					wattron(win, COLOR_PAIR(AIR));
					mvwaddstr(win, y, x, zo_sym[sign]);
					wattroff(win, COLOR_PAIR(AIR));
					break;
				case WATER:
					wattron(win, COLOR_PAIR(WATER));
					mvwaddstr(win, y, x, zo_sym[sign]);
					wattroff(win, COLOR_PAIR(WATER));
					break;
			}
			break;
		case 'd':
			switch(j)
			{
					case FIRE:
					wattron(win, COLOR_PAIR(FIRE));
					mvwprintw(win, y, x, "%.2f", p_arr[count][MWIN]);
					wattroff(win, COLOR_PAIR(FIRE));
					break;
				case EARTH:
					wattron(win, COLOR_PAIR(EARTH));
					mvwprintw(win, y, x, "%.2f", p_arr[count][MWIN]);
					wattroff(win, COLOR_PAIR(EARTH));
					break;
				case AIR:
					wattron(win, COLOR_PAIR(AIR));
					mvwprintw(win, y, x, "%.2f", p_arr[count][MWIN]);
					wattroff(win, COLOR_PAIR(AIR));
					break;
				case WATER:
					wattron(win, COLOR_PAIR(WATER));
					mvwprintw(win, y, x, "%.2f", p_arr[count][MWIN]);
					wattroff(win, COLOR_PAIR(WATER));
					break;
			}
			break;
	}
}

void draw_circle(WINDOW *main_win, int radius, chtype ch)
{
	int center_x = COLS / 2;
	int center_y = LINES / 2;
	
	int x = 0;
	int y = radius;
	int d = 3 -2 * radius;
	
	while (x <= y)
	{
		mvwaddch(main_win, center_y + y / 2, center_x + x, ch);
		mvwaddch(main_win, center_y + y / 2, center_x - x, ch);
		mvwaddch(main_win, center_y - y / 2, center_x + x, ch);
		mvwaddch(main_win, center_y - y / 2, center_x - x, ch);
		
		mvwaddch(main_win, center_y + x / 2, center_x + y, ch);
		mvwaddch(main_win, center_y + x / 2, center_x - y, ch);
		mvwaddch(main_win, center_y - x / 2, center_x + y, ch);
		mvwaddch(main_win, center_y - x / 2, center_x - y, ch);
	
		if (d < 0)
			d = d + 4 * x + 6;
		else
		{
			d = d + 4 * (x - y) + 10;
			y--;
		}
		x++;
	}
}

void planet_pos(WINDOW *main_win, int radius, Pxx *pxx)
{ // help wanted: structured, cleanly, planet collision offset
	double asc = cusps[1];
	
	double *p_arr[] = {
		pxx->dsun, pxx->dmoon,
		pxx->dmerc, pxx->dven,
		pxx->dmars, pxx->djup,
		pxx->dsat, pxx->dura,
		pxx->dnep, pxx->dplu,
		pxx->dmnod, pxx->dtnod};
		
	int center_x = (COLS / 2);
	int center_y = (LINES / 2);

	for (int i = 0; i < 12; ++i)
	{
		double rad = (p_arr[i][LONG] - asc) * M_PI / 180.0;
		
		int x = center_x - (int)(radius * cos(rad));
		int y = center_y + (int)(radius * sin(rad) * 0.5);
		
		int offsety = 0;
		int offsetx = 0;
		/* 	cos	1	0	-1	0
				0	90	180	270
			sin	0	1	0	-1
		*/
		int dir_x = ((int)cos(rad) != 0) ? 1 : -1;
		int dir_y = ((int)sin(rad) != 0) ? -1 : 1;
		
		bool near_horizontal = (fabs(sin(rad)) < 0.8);

		for (int j = 0; j < i; j++)
		{
			double adj_angle = p_arr[j][LONG];
			double ang_dist = fabs(p_arr[i][LONG] - adj_angle);
			
			if (ang_dist <= 8 || ang_dist >= 352)
			{
				if (near_horizontal)
				{
					offsetx += 7;
					offsety += 2;
				}
				else
				{
					offsety -= 4;
					offsetx -= 3;
				}
			}
		}
		
		for (int j = 0; j < i; j++)
		{
			double adj_angle = p_arr[j][LONG];
			double ang_dist = fabs(p_arr[i][LONG] - adj_angle);
			if (ang_dist <= 8 || ang_dist >= 352)
			{
				if (near_horizontal)
				{
					if ((fabs(p_arr[j][LONG] - pxx->dasc)) < 30)
					{
						offsetx -= 13;
						offsety -= 2;
					}
					else
					{
						offsetx += 3;
						offsety -= 2;
					}
				}
				else
				{
					offsetx += 5;
					offsety += 2;
				}
			}
		}
		
		if (!near_horizontal)
			offsety = dir_y * offsety;
		
		offsetx = dir_x * offsetx;
		
		double decimal  = (((p_arr[i][LONG] - (int)p_arr[i][LONG]) * 60) / 100);
		
		int sign = ((int)p_arr[i][LONG] / 30) + 1;
		
		p_arr[i][MWIN] = ((int)p_arr[i][LONG] % 30) + decimal;
		
		if (i != SE_MEAN_NODE)
		{
			element_color(main_win, (y + offsety) -1, (x + offsetx), i, sign, pxx, 'd');
		
			mvwaddstr(main_win, y + offsety, x + offsetx, pl_sym[i]);
		}
	}
}

void ascmc_pos(WINDOW *main_win, int radius)
{
	for (int i = 0; i < 2; ++i)
	{
		const char *ascmc_sym[] = {"as", "mc"};
		int center_x = (COLS / 2);
		int center_y = (LINES / 2);
		
		double rad = (ascmc[i] - cusps[1]) * M_PI / 180.0;
		
		int x = center_x - (int)(radius * cos(rad));
		int y = center_y + (int)(radius * sin(rad) * 0.5);
		
		if (i == 0) // draw asc line
		{
			for (int r = 0; r <= radius; r++)
			{
				int line_x = center_x - (int)(r * cos(rad));
				int line_y = center_y + (int)(r * sin(rad) * 0.5);
				
				if (line_x >= 0 && line_x < COLS
				&& line_y >= 0 && line_y < LINES)
					mvwaddch(main_win, line_y, line_x, '`');
			}
		}
		
		mvwaddstr(main_win, y, x, ascmc_sym[i]);
		
		double decimal = (((ascmc[i] - (int)ascmc[i]) * 60) / 100);
		
		char buffer[56];
		snprintf(buffer, sizeof(buffer), "%.2f", ((int)ascmc[i] % 30) + 
		decimal);
		
		mvwaddstr(main_win, y - 1, x, buffer);
	}
}

void zo_pos(WINDOW *main_win, int radius, Pxx *pxx)
{
	int asc_sign = (int)(pxx->dasc / 30);
	for (int i = 1; i < 13; ++i)
	{
		int sign = ((i + asc_sign - 1) % 12);
		if (sign == 0)
			sign = 12;

		int center_x = (COLS / 2);
		int center_y = (LINES / 2);
		
		int sign_inc = (((int)pxx->dasc / 30) * 30) + 15;
		
		double rad = (cusps[i] - sign_inc) * M_PI / 180.0;
		
		int x = center_x - (int)(radius * cos(rad));
		int y = center_y + (int)(radius * sin(rad) * 0.5);
		
		element_color(main_win, y, x, i, sign, pxx, 'z');
	}
}

void draw_house(WINDOW *main_win,
int radius, chtype ch)
{
	for (int i = 0; i < 13; ++i)
	{
		double rad = cusps[i] * M_PI / 180.0;
		
		int center_x = COLS / 2;
		int center_y = LINES / 2;
		
		int edge_x = center_x - (int)(radius * cos(rad));
		int edge_y = center_y + (int)(radius * sin(rad) * 0.5);
		
		int half_x = center_x - (int)((radius / 2) * cos(rad));
		int half_y = center_y + (int)((radius / 2)  * sin(rad) * 0.5);
		
		int dx = edge_x - half_x;
		int dy = edge_y - half_y;
		
		int distance = (int)sqrt(dx * dx + dy * dy);
		if (distance == 0)
			distance = 1;
		
		for(int j = 0; j <= distance; j++)
		{
			int x = half_x + (dx * j) / distance;
			int y = half_y + (dy * j) / distance;
			mvwaddch(main_win, y, x, ch);
		}
	}
}

int sect(Pxx *pxx)
{
	int sect = NIGHT_SECT;
	
	if ((pxx->dsun[LONG] - pxx->dasc) <= 180)
		sect = DAY_SECT;
	else
		sect = NIGHT_SECT;
	return sect;
}

void lots(int sect, Pxx *pxx)
{
	double diff;
	
	if (sect == DAY_SECT)
	{
		diff = pxx->dmoon[LONG] - pxx->dsun[LONG];
		pxx->dfor = pxx->dasc - diff;
		pxx->dspir = pxx->dasc + diff;
	}
	else // night
	{
		diff = pxx->dmoon[LONG] - pxx->dsun[LONG];
		pxx->dfor = pxx->dasc + diff;
		pxx->dspir = pxx->dasc - diff;
	}
	
	pxx->dfor = fmod(pxx->dfor, 360.0);
	if (pxx->dfor < 0.0)
		pxx->dfor += 360.0;
	pxx->dspir = fmod(pxx->dspir, 360.0);
	if (pxx->dspir < 0.0)
		pxx->dspir += 360.0;
}

void check_dst(Cdata *cdata)
{
	struct tm tm_in = {0};
	tm_in.tm_year = cdata->tm_year - 1900;
	tm_in.tm_mon = cdata->tm_mon - 1;
	tm_in.tm_mday = cdata->tm_mday;
	tm_in.tm_hour = cdata->tm_hour;
	tm_in.tm_min = cdata->tm_min;
	tm_in.tm_sec = cdata->tm_sec;
	tm_in.tm_isdst = -1;
	
	time_t t = mktime(&tm_in);
	struct tm *result = localtime(&t);
	
	cdata->tm_isdst = result->tm_isdst;
	
	struct tm *utc_tm = gmtime(&t);
	
	int offset_hours = tm_in.tm_hour - utc_tm->tm_hour;
	
	if (tm_in.tm_mday != utc_tm->tm_mday)
	{
		if (tm_in.tm_mday > utc_tm->tm_mday)
			offset_hours += 24;
		else
			offset_hours -= 24;
	}
	
	cdata->utc_off = offset_hours;
}

void chart_timeset(Cdata *cdata, int *day_offset)
{
	check_dst(cdata);
	
	double utc_offset = (double)cdata->utc_off;
	
	double min = (double)cdata->tm_min / 60;
	double sec = (double)cdata->tm_sec / 3600.0;
	
	double dhour = ((double)(cdata->tm_hour - utc_offset) + min  ) + sec;
	
	if((dhour >= 24.0))
	{
		dhour -= 24.0;
		++cdata->tm_mday;
		*day_offset -= 1;
	}
	else if((dhour <= 0))
	{
		dhour += 24.0;
		--cdata->tm_mday;
		*day_offset += 1;
	}
	
	cdata->dhour = dhour; 
}

void pxx_fill(Cdata *cdata, Pxx *pxx)
{
	int day_offset = 0;
	int iret;
	size_t i;
	
	double *pxx_members[] = {
	pxx->dsun, pxx->dmoon,
	pxx->dmerc, pxx->dven,
	pxx->dmars, pxx->djup,
	pxx->dsat, pxx->dura,
	pxx->dnep, pxx->dplu,
	pxx->dmnod, pxx->dtnod,
	&pxx->dasc, &pxx->dmc,
	&pxx->ddsc, &pxx->dic,
	&pxx->dfor, &pxx->dspir};
	
	chart_timeset(cdata, &day_offset); // goes before swe_julday
	
	double jul_day_UT = swe_julday(cdata->tm_year, cdata->tm_mon, 
	cdata->tm_mday, cdata->dhour, SE_GREG_CAL);
	
	// corrects cdata->dhour offset from chart_timeset()
	cdata->tm_mday += day_offset;

	iflag = SEFLG_SWIEPH | SEFLG_SPEED;
	for (ipl = SE_SUN, i = 0; ipl <= SE_TRUE_NODE; ipl++, i++)
	{
	
		iret = swe_calc_ut(jul_day_UT, ipl, iflag, xx, serr);
		if (iret < 0) 
			ERR_EXIT("ERR: swe_calc_ut failure");
			
		pxx_members[i][LONG] = xx[LONG];
		pxx_members[i][LAT] = xx[LAT];
		pxx_members[i][DIST] = xx[DIST];
		pxx_members[i][LONG_S] = xx[LONG_S];
		pxx_members[i][LAT_S] = xx[LAT_S];
		pxx_members[i][DIST_S] = xx[DIST_S];
	}
	
	iret = swe_houses_ex(jul_day_UT, 0, cdata->dlat, cdata->dlon,
	ihsy, cusps, ascmc);
	if (iret < 0)
		ERR_EXIT("ERR: swe_houses_ex failure");
	
	// calculates ic/mc and fills struct members
	double asc = ascmc[0];
	double dsc = (ascmc[0] + 180);
	if (dsc >= 360)
		dsc -= 360;
	double ic = (ascmc[1] + 180);
	if (ic >= 360)
		ic -= 360;
	double mc = ascmc[1];
	
	pxx->dasc = asc;
	pxx->ddsc = dsc;
	pxx->dic = ic;
	pxx->dmc = mc;
	
	int chart_sect = sect(pxx);
	lots(chart_sect, pxx);
}

void draw_chart(WINDOW *main_win, Pxx *pxx)
{
	curs_set(0);
	wclear(main_win);
	int radius = ((COLS / 2 < LINES) ? COLS / 2 : LINES) - 5;
	
	// zodiac
	draw_circle(main_win, radius + 4, '`');
	// out
	draw_circle(main_win, radius, '.');
	// in
	draw_circle(main_win, (radius / 2) - 1, '.');
	
	draw_house(main_win, radius + 4, '`');
	
	zo_pos(main_win, radius + 3, pxx);
	
	planet_pos(main_win, radius - 9, pxx);
	
	ascmc_pos(main_win, (radius / 2) + 4);
}

void cur_chart_data(WINDOW *main_win, Io *io, 
Cdata *cdata)
{	
	int starty = 3;
	int startx = COLS - 22;
	
	if(io->filename)
		mvwprintw(main_win, starty, startx, "%s", io->filename);
	
	starty += 1;
	if(cdata->city)
		mvwprintw(main_win, starty, startx, "%s", cdata->city);
	
	starty += 1;
	if(cdata->tm_year)
		mvwprintw(main_win, starty, startx, "%d", cdata->tm_year);
	
	starty += 1;
	if(cdata->tm_mon && cdata->tm_mday)
		mvwprintw(main_win, starty, startx, "%d/%d",
		cdata->tm_mon, cdata->tm_mday);
		
	starty += 1;
	if (cdata->tm_hour >= 0)
	{
		if (cdata->tm_hour <= 9)
			mvwprintw(main_win, starty, startx, "0%d",
			cdata->tm_hour);
		else
			mvwprintw(main_win, starty, startx, "%d",
			cdata->tm_hour);
	}
	
	if(cdata->tm_min >= 0)
	{
		startx += 2;
		if (cdata->tm_min == 0)
			mvwprintw(main_win, starty, startx, ":%d0",
			cdata->tm_min);
		else if (cdata->tm_min <= 9)
			mvwprintw(main_win, starty, startx, ":0%d",
			cdata->tm_min);
		else
			mvwprintw(main_win, starty, startx, ":%d",
			cdata->tm_min);
	}
	
	if (cdata->tm_sec >= 0)
	{
		startx += 3;
		if (cdata->tm_sec == 0)
			mvwprintw(main_win, starty, startx, ":%d0",
			cdata->tm_sec);
		else if (cdata->tm_sec <= 9)
			mvwprintw(main_win, starty, startx, ":0%d",
			cdata->tm_sec);
		else
			mvwprintw(main_win, starty, startx, ":%d",
			cdata->tm_sec);
		startx -= 5;
	}
	
	starty += 1;
	if (fabs(cdata->dlat) > 1e-6)
		mvwprintw(main_win, starty, startx, "lat.%f", cdata->dlat);
	
	starty += 1;
	if (fabs(cdata->dlon) > 1e-6)
		mvwprintw(main_win, starty, startx, "lon.%f", cdata->dlon);
}

void planet_table(WINDOW *planet_win, Pxx *pxx)
{
	char spname[AS_MAXCH];
	int p_count = 18;
	
	double *p_arr[] = {
		pxx->dsun, pxx->dmoon,
		pxx->dmerc, pxx->dven,
		pxx->dmars, pxx->djup,
		pxx->dsat, pxx->dura,
		pxx->dnep, pxx->dplu,
		pxx->dmnod, pxx->dtnod,
		&pxx->dfor, &pxx->dspir,
		&pxx->dasc, &pxx->dmc,
		&pxx->ddsc, &pxx->dic};
		
	for (int i = 0; i < PWINY; i++) 
	    mvwhline(planet_win, i, 0, ' ', PWINX);
	
	int starty = 1, startx = 2;
	int j = 0;
	
	for (int i = 0; i < p_count; ++i)
	{
		int sign = ((int)p_arr[i][LONG] / 30) + 1;
		
		int deg = (int)p_arr[i][LONG] % 30;
		double dec = (((p_arr[i][LONG] - (int)p_arr[i][LONG]) * 60) / 100);
		int a_dec = (int)(dec * 100) % 100;
		
		int full_deg = (int)p_arr[i][LONG];
		double full_dec = (((p_arr[i][LONG] - (int)p_arr[i][LONG]) * 60) / 100);
		int a_full_dec = (int)(full_dec * 100) % 100;
		
		if ( i != SE_MEAN_NODE && i < 12) // sun -> node 
		{
			swe_get_planet_name(i, spname);
			spname[2] ='\0';
			
			char buff[MAXBUF];
			
			snprintf(buff, sizeof(buff),
			"%-3s %3d.%-2d : %6s %2d\xc2\xb0%d`",
			spname, full_deg, a_full_dec,
			pl_sym[i], deg, a_dec);
			
			mvwprintw(planet_win, starty, startx, "%s ", buff);
			
			startx += (int)strlen(buff);
			element_color(planet_win, starty, startx + 1, 1, sign, pxx, 'z');
			startx -= (int)strlen(buff);
			
			starty += 2;
		}
		
		else if ( i != SE_MEAN_NODE && i >= 12) // asc -> ic
		{
			const char *points[] = {"for", "spi", "asc", "mc", "dsc", "ic"};
			char point_buff[MAXBUF];
			
			snprintf(point_buff, sizeof(point_buff),
			"%-11s %3d.%-2d : %2d\xc2\xb0%d` ",
			points[j], full_deg, a_full_dec, deg, a_dec);
			
			if (i == 12) // lots divider
			{
				mvwprintw(planet_win, starty, startx,
				"------------------------------");
				starty += 2;
			}
			
			if (i == 14) // points divider
			{
				mvwprintw(planet_win, starty, startx,
				"------------------------------");
				starty += 2;
			}
			mvwprintw(planet_win, starty, startx, "%s", point_buff);
			
			startx += (int)strlen(point_buff);
			element_color(planet_win, starty, startx + 1, 1, sign, pxx, 'z');
			startx -= (int)strlen(point_buff);
	
			starty += 2;
			++j;
		}
	}
}

void retrograde_table(WINDOW *retro_win, Pxx *pxx)
{
	double *p_arr[] = {
		pxx->dmerc, pxx->dven,
		pxx->dmars, pxx->djup,
		pxx->dsat, pxx->dura,
		pxx->dnep, pxx->dplu};
		
	size_t p_count = 8;
		
	for (int i = 0; i < RWINY; i++) 
	    mvwhline(retro_win, i, 0, ' ', RWINX);
	    
	for (size_t i = 0; i < p_count; ++i)
	{
		char buff[MAXBUF];
		
		snprintf(buff, sizeof(buff), "%-6s %-6f",
		pl_sym[i+2], p_arr[i][LONG_S]);
		
		mvwprintw(retro_win, (int)i, 0, "%s", buff);
	}
}

void new_chart(WINDOW *main_win, WINDOW *planet_win, WINDOW *retro_win,
PANEL **planet_panel, PANEL **retro_panel,
Io *io, Cdata *cdata, Pxx *pxx,
int *planet_trig, int *retro_trig)
{
	pxx_fill(cdata, pxx);
	draw_chart(main_win, pxx);
	cur_chart_data(main_win, io, cdata);
	wrefresh(main_win);
	
	if (*planet_trig > 0)
	{
		planet_table(planet_win, pxx);
		show_panel(*planet_panel);
		update_panels();
	}
	if (*retro_trig > 0)
	{
		retrograde_table(retro_win, pxx);
		show_panel(*retro_panel);
		update_panels();
	}	
}

void realtime_chart(WINDOW *main_win, WINDOW *planet_win, WINDOW *retro_win,
PANEL **planet_panel, PANEL **retro_panel,
Io *io, Cdata *cdata, Pxx *pxx,
int *planet_trig, int *retro_trig)
{
	nodelay(main_win, TRUE);
	struct tm gettime = {0};
		
	int ch = 0;
	while ((ch = wgetch(main_win)) != 9)
	{
		wattron(main_win, COLOR_PAIR(FIRE));
		mvwprintw(main_win, 2, COLS - 22, "*live");
		wattroff(main_win, COLOR_PAIR(FIRE));
		wrefresh(main_win);
		
		for (int i = 0; i < 10; ++i)
		{
			usleep(100000);
			if ((ch = wgetch(main_win)) == 9)
				break;
		}
		
		if (ch == 9)
			break;
		
		time_t now = time(NULL);
		localtime_r(&now, &gettime);
		
		cdata->tm_year = gettime.tm_year+1900;
		cdata->tm_mon = gettime.tm_mon + 1;
		cdata->tm_mday = gettime.tm_mday;
		cdata->tm_hour = gettime.tm_hour;
		cdata->tm_min = gettime.tm_min;
		cdata->tm_sec = gettime.tm_sec;
		
		new_chart(main_win, planet_win, retro_win,
		planet_panel, retro_panel,
		io, cdata, pxx,
		planet_trig, retro_trig);
		
		update_panels();
		doupdate();
	}
	wmove(main_win, 2, COLS - 22);
	wclrtoeol(main_win);
	nodelay(main_win, FALSE);
}
	
void animate_chart(WINDOW *main_win, WINDOW *planet_win, WINDOW *retro_win,
PANEL **planet_panel, PANEL **retro_panel,
Io *io, Cdata *cdata, Pxx *pxx,
int *planet_trig, int *retro_trig)
{
	int starty = 11;
	int startx = COLS - 22;
	
	mvwprintw(main_win, starty, startx, "(min)");
	
	int max_day = 0; // months() return flag
	size_t i = MINUTE; // time inc/dec
	
	int ch = 0;
	int anim_done = 0;
	while(!anim_done && (ch = wgetch(main_win)))
	{
		switch(ch)
		{
			case 'h': case KEY_LEFT:
				if (i != MINUTE)
					++i;
				break;
				
			case 'l': case KEY_RIGHT:
				if (i != YEAR) // time inc/dec
					--i;
				break;
				
			case 'p':
				if (*planet_trig)
				{
					hide_panel(*planet_panel);
					clear();
					refresh();
					*planet_trig = 0;
				}
				else
				{
					planet_table(planet_win, pxx);
					show_panel(*planet_panel);
					*planet_trig = 1;
				}
				
				if (*retro_trig > 0)
				{
					retrograde_table(retro_win, pxx);
					show_panel(*retro_panel);
				}
				
				touchwin(main_win);
				wrefresh(main_win);
				update_panels();
				doupdate();
				break;
				
			case 'o':
				if (*retro_trig)
				{
					hide_panel(*retro_panel);
					clear();
					refresh();
					*retro_trig = 0;
				}
				else
				{
					retrograde_table(retro_win, pxx);
					show_panel(*retro_panel);
					*retro_trig = 1;
				}
				
				if (*planet_trig > 0)
				{
					planet_table(planet_win, pxx);
					show_panel(*planet_panel);
				}
				touchwin(main_win);
				wrefresh(main_win);
				update_panels();
				doupdate();
				break;
			
			case 'k': case KEY_UP:
				switch(i)
				{
					case MINUTE:
						if ((++cdata->tm_min) > 59)
						{
							++cdata->tm_hour;
							cdata->tm_min = 0;
							if ((cdata->tm_hour) > 23)
							{
								cdata->tm_hour = 0;
								++cdata->tm_mday;
								if (cdata->tm_mday > months(
								cdata->tm_mon, cdata->tm_year))
								{
									cdata->tm_mday = 1;
									++cdata->tm_mon;
									if (cdata->tm_mon > 12)
									{
										cdata->tm_mon = 1;
										++cdata->tm_year;
									}
								}
							}
						}
						new_chart(main_win, planet_win, retro_win,
						planet_panel, retro_panel,
						io, cdata, pxx, 
						planet_trig, retro_trig);
						break;
						
					case HOUR:
						if ((++cdata->tm_hour) > 23)
						{
							cdata->tm_hour = 0;
							++cdata->tm_mday;
							if (cdata->tm_mday > months(
							cdata->tm_mon, cdata->tm_year))
							{
								cdata->tm_mday = 1;
								++cdata->tm_mon;
								if (cdata->tm_mon > 12)
								{
									cdata->tm_mon = 1;
									++cdata->tm_year;
								}
							}
						}
						new_chart(main_win, planet_win, retro_win,
						planet_panel, retro_panel,
						io, cdata, pxx, 
						planet_trig, retro_trig);
	
						break;
						
					case DAY:
						if ((++cdata->tm_mday) > months(
						cdata->tm_mon, cdata->tm_year))
						{
							cdata->tm_mday = 1;
							++cdata->tm_mon;
							if (cdata->tm_mon > 12)
							{
								cdata->tm_mon = 1;
								++cdata->tm_year;
							}
						}
						new_chart(main_win, planet_win, retro_win,
						planet_panel, retro_panel,
						io, cdata, pxx, 
						planet_trig, retro_trig);
	
						break;
						
					case MONTH:
						if ((++cdata->tm_mon) > 12)
						{
							cdata->tm_mon = 1;
							++cdata->tm_year;
						}
						max_day = months(
						cdata->tm_mon, cdata->tm_year);
						if (cdata->tm_mday > max_day)
							cdata->tm_mday = max_day;
							
						new_chart(main_win, planet_win, retro_win,
						planet_panel, retro_panel,
						io, cdata, pxx, 
						planet_trig, retro_trig);
	
						break;
						
					case YEAR:
						if ((++cdata->tm_year) > 16799)
							cdata->tm_year = -12998;
							
						new_chart(main_win, planet_win, retro_win,
						planet_panel, retro_panel,
						io, cdata, pxx, 
						planet_trig, retro_trig);
	
						break;
				}
				break;
			case 'j': case KEY_DOWN:
				switch(i)
				{
					case MINUTE:
						if ((--cdata->tm_min) < 0)
						{
								--cdata->tm_hour;
							cdata->tm_min = 59;
							if ((cdata->tm_hour) < 0)
							{
								cdata->tm_hour = 23;
								--cdata->tm_mday;
								if (cdata->tm_mday < 1)
								{
									--cdata->tm_mon;
									if (cdata->tm_mon < 1)
									{
										cdata->tm_mon = 12;
										--cdata->tm_year;
									}
									cdata->tm_mday = months(
									cdata->tm_mon, cdata->tm_year);
								}
							}
						}
						new_chart(main_win, planet_win, retro_win,
						planet_panel, retro_panel,
						io, cdata, pxx, 
						planet_trig, retro_trig);
	
						break;
						
					case HOUR:
						if ((--cdata->tm_hour) < 0)
						{
							cdata->tm_hour = 23;
							--cdata->tm_mday;
							if (cdata->tm_mday < 1)
							{
								--cdata->tm_mon;
								if (cdata->tm_mon < 1)
								{
									cdata->tm_mon = 12;
									--cdata->tm_year;
								}
								cdata->tm_mday = months(
								cdata->tm_mon, cdata->tm_year);
							}
						}
						new_chart(main_win, planet_win, retro_win,
						planet_panel, retro_panel,
						io, cdata, pxx, 
						planet_trig, retro_trig);
	
						break;
						
					case DAY:
						if ((--cdata->tm_mday) < 1)
						{
							--cdata->tm_mon;
							if (cdata->tm_mon < 1)
							{
								cdata->tm_mon = 12;
								--cdata->tm_year;
							}
							cdata->tm_mday = months(
							cdata->tm_mon, cdata->tm_year);
						}
						new_chart(main_win, planet_win, retro_win,
						planet_panel, retro_panel,
						io, cdata, pxx, 
						planet_trig, retro_trig);
	
						break;
						
					case MONTH:
						if (--cdata->tm_mon < 1)
						{
							cdata->tm_mon = 12;
							--cdata->tm_year;
						}
						max_day = months(
						cdata->tm_mon, cdata->tm_year);
						if (cdata->tm_mday > max_day)
							cdata->tm_mday = max_day;
							
						new_chart(main_win, planet_win, retro_win,
						planet_panel, retro_panel,
						io, cdata, pxx, 
						planet_trig, retro_trig);
							
						break;
						
					case YEAR:
						if ((--cdata->tm_year) < -12998)
							cdata->tm_year = 16799;
							
						new_chart(main_win, planet_win, retro_win,
						planet_panel, retro_panel,
						io, cdata, pxx, 
						planet_trig, retro_trig);
	
						break;
				}
				break;
			case '\n':
				anim_done = 1;
				break;
		}
		
		switch(i)
		{
			case MINUTE:
				wmove(main_win, starty, startx);
				wclrtoeol(main_win);
				mvwprintw(main_win, starty, startx, "(min)");
				break;
				
			case HOUR:
				wmove(main_win, starty, startx);
				wclrtoeol(main_win);
				mvwprintw(main_win, starty, startx, "(hour)");
				break;
				
			case DAY:
				wmove(main_win, starty, startx);
				wclrtoeol(main_win);
				mvwprintw(main_win, starty, startx, "(day)");
				break;
				
			case MONTH:
				wmove(main_win, starty, startx);
				wclrtoeol(main_win);
				mvwprintw(main_win, starty, startx, "(mon)");
				break;
				
			case YEAR:
				wmove(main_win, starty, startx);
				wclrtoeol(main_win);
				mvwprintw(main_win, starty, startx, "(year)");
				break;
		}
	}
	wmove(main_win, starty, startx);
	wclrtoeol(main_win);
	wrefresh(main_win);
}

int main()
{
	Cdata *cdata = calloc(1, sizeof(Cdata));
	if (!cdata)
		ERR_EXIT("main Location calloc");
	cdata->city = malloc(MAXBUF);
	if (!cdata->city)
		ERR_EXIT("ERR: main cdata->city malloc");
		
	char *citybuffer = malloc(MAXBUF);
	if (!citybuffer)
		ERR_EXIT("ERR: main citybuffer alloc fail");
	
	Pxx *pxx = calloc(1, sizeof(Pxx));
	if (!pxx)
		ERR_EXIT("main pxx");
	
	// xx[] lat, long, dist, lat_s, long_s, dist_s
	pxx->dsun   = calloc(MAXPXX, sizeof(double));                                  
	if(!pxx->dsun)
		ERR_EXIT("pxx->d calloc");
	pxx->dmoon  = calloc(MAXPXX, sizeof(double));                                  
	if(!pxx->dmoon)
		ERR_EXIT("pxx->d calloc");
	pxx->dmerc  = calloc(MAXPXX, sizeof(double));                                  
	if(!pxx->dmerc)
		ERR_EXIT("pxx->d calloc");
	pxx->dven   = calloc(MAXPXX, sizeof(double));                                  
	if(!pxx->dven)
		ERR_EXIT("pxx->d calloc");
	pxx->dmars  = calloc(MAXPXX, sizeof(double));                                  
	if(!pxx->dmars)
		ERR_EXIT("pxx->d calloc");
	pxx->djup   = calloc(MAXPXX, sizeof(double));                                  
	if(!pxx->djup)
		ERR_EXIT("pxx->d calloc");
	pxx->dsat   = calloc(MAXPXX, sizeof(double));                                  
	if(!pxx->dsat)
		ERR_EXIT("pxx->d calloc");
	pxx->dura   = calloc(MAXPXX, sizeof(double));                                  
	if(!pxx->dura)
		ERR_EXIT("pxx->d calloc");
	pxx->dnep   = calloc(MAXPXX, sizeof(double));                                  
	if(!pxx->dnep)
		ERR_EXIT("pxx->d calloc");
	pxx->dplu   = calloc(MAXPXX, sizeof(double));                                  
	if(!pxx->dplu)
		ERR_EXIT("pxx->d calloc");
	pxx->dmnod  = calloc(MAXPXX, sizeof(double));                                 
	if(!pxx->dmnod)
		ERR_EXIT("pxx->d calloc");
	pxx->dtnod  = calloc(MAXPXX, sizeof(double)); 
	if(!pxx->dtnod)
		ERR_EXIT("pxx->d calloc");
		
	Io *io = calloc(1, sizeof(Io));
	if (!io)
		ERR_EXIT("mai io calloc");
	io->filepath = malloc(MAXBUF);
	if (!io->filepath)
		ERR_EXIT("main io->filepath malloc");
	io->filename = malloc(256);
	if (!io->filename)
		ERR_EXIT("main io->filename malloc");
	
	struct passwd *pw = getpwuid(getuid());
	if (!pw)
		ERR_EXIT("getpwuid main");
		
	char fn_buff[MAXBUF] = {0};
	snprintf(fn_buff, MAXBUF, 
	"%s/.local/share/astro/ephe", pw->pw_dir);
	
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
	
	int main_done = 0;
	while (!main_done)
	{
		input_chart_data(io, cdata, citybuffer);
		cdata->city = citybuffer;
		pxx_fill(cdata, pxx);
		draw_chart(main_win, pxx);
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
					&planet_trig, &retro_trig);
					break;
			case 9: // tab
					realtime_chart(main_win, planet_win, retro_win,
					&planet_panel, &retro_panel,
					io, cdata, pxx,
					&planet_trig, &retro_trig);
					break;
				case 'q':
					main_done = 1;
					chart_done = 1;
					break;
				case 'i':
					wclear(main_win);
					wrefresh(main_win);
					free(io->filename);
					io->filename = NULL;
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
					wrefresh(main_win);
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
					wrefresh(main_win);
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
