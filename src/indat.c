// Copyright (C) 2026 yam lynn
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTIBILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License for more details.

// You should have received a copy of the GNU Affero General Public License
// along with this program. if not, see <https://www.gnu.org/licenses/>

#include <ncurses.h>
#include <strings.h>
#include <form.h>
#include <errno.h>
#include <time.h>
#include "swephexp.h"
#include "astro.h"
#include "io.h"
#include "indat.h"
#include "chronos.h"
#include "draw.h"
#include "search.h"

static void setfield_localtime(FIELD *cdata_field[], struct cdata *cdata)
{
	char buff[MAXBUF] = {0};
	if ((setenv("TZ", cdata->timezone, 1) != 0))
		return;
	tzset();
	
	struct tm *gettime = ecalloc(1,sizeof(struct tm));
		
	time_t now = time(NULL);
	localtime_r(&now, gettime);
	
	if (gettime->tm_hour == 0)
		gettime->tm_hour = 12;
	
	if (gettime->tm_hour > 12)
	{
		gettime->tm_hour -= 12;
		set_field_buffer(cdata_field[AMPM], 0, "pm");
	}
	else
		set_field_buffer(cdata_field[AMPM], 0, "am");
	
	snprintf(buff, sizeof(buff), "%d", gettime->tm_year+1900);
	set_field_buffer(cdata_field[YEAR], 0, buff);
	
	snprintf(buff, sizeof(buff), "%d", gettime->tm_mon + 1);
	set_field_buffer(cdata_field[MONTH], 0, buff);
	
	snprintf(buff, sizeof(buff), "%d", gettime->tm_mday);
	set_field_buffer(cdata_field[DAY], 0, buff);
	
	snprintf(buff, sizeof(buff), "%d", gettime->tm_hour);
	set_field_buffer(cdata_field[HOUR], 0, buff);
	
	snprintf(buff, sizeof(buff), "%d", gettime->tm_min);
	set_field_buffer(cdata_field[MINUTE], 0, buff);
	
	snprintf(buff, sizeof(buff), "%d", gettime->tm_sec);
	set_field_buffer(cdata_field[SECOND], 0, buff);
	
	free(gettime);
}

static void buff_trim(FIELD *current, char *buffer)
{
	char *f = field_buffer(current, 0);
	int len = 0;
	field_info(current, NULL, &len, NULL, NULL, NULL, NULL);
	
	if (len <= 0)
	{
		buffer[0] = '\0';
		return;
	}
	
	memcpy(buffer, f, (size_t)len);
	
	while(len > 0 && buffer[len - 1] == ' ')
		--len;
	buffer[len] = '\0';
}

static void field_to_member (struct cdata *cdata, char xdg_path[], FORM *cdata_form, FIELD *cdata_field[])
{
	FIELD *current = current_field(cdata_form);
	int index = field_index(current);
	
	char *endptr = NULL;
	long iret;
	double dret;
	errno = 0;
	
	char buffer[MAXBUF] = {0};
	buff_trim(current, buffer);
	
	switch(index)
	{
		case CITY:
			city_search(cdata_field, cdata_form, buffer, cdata, xdg_path);
			form_driver(cdata_form, REQ_VALIDATION);
			
			buff_trim(current, buffer);
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
			if (errno != ERANGE && iret != -1 && iret <= 12) 
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
			
		case SECOND:
			iret = strtol(buffer, &endptr, 10);
			if (errno != ERANGE && iret != -1)
				cdata->tm_sec = (int)iret;
			else
				cdata->tm_sec = 1;
			break;
		
		case AMPM:
			if ((!strcasecmp(buffer, "p") || !strcasecmp(buffer, "pm"))
			&& cdata->tm_hour != 12)
				cdata->tm_hour += 12;
			if (cdata->tm_hour >= 24)
				cdata->tm_hour = 0;
			break;
			
		case TIMEZONE:
			if (setenv("TZ", buffer, 1) != 0)
				ERR_EXIT("ERR: TZ setenv fail field_to_member");
			memcpy(cdata->timezone, buffer, strlen(buffer) + 1);
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
}

static void validate_fields(FIELD *cdata_field[], FORM *cdata_form, struct cdata *cdata, char xdg_path[])
{
	set_current_field(cdata_form, cdata_field[0]);
	FIELD *current = current_field(cdata_form);
	char buffer[MAXBUF] = {0};
	buff_trim(current, buffer);
		
	for (int i = 1; i < FIELDMAX; i++)
	{
		set_current_field(cdata_form, cdata_field[i]);
		form_driver(cdata_form, REQ_VALIDATION);
		field_to_member(cdata, xdg_path, cdata_form, cdata_field);
	}
}

static void clear_fields(FIELD *cdata_field[], FORM *cdata_form)
{
	for (int i = 0; i < FIELDMAX; i++)
	{
		set_current_field(cdata_form, cdata_field[i]);
		form_driver(cdata_form, REQ_CLR_FIELD);
	}
	set_current_field(cdata_form, cdata_field[CITY]);
}

static void field_label(WINDOW *in_cdata_win)
{
	const char *labels[] = {
		"city search:",
		"year:",
		"month:",
		"day:",
		"hour:",
		"minute:",
		"second:",
		"am/pm:",
		"timezone:",
		"lat.",
		"long.",
		NULL
	};
	
	int starty = 1;
	int startx = 1;
	
	for (size_t i = CITY; i < FIELDMAX; ++i, starty += 2)
			mvwprintw(in_cdata_win, starty, startx, "%s", labels[i]);
			
	box(in_cdata_win, 0, 0);
	wrefresh(in_cdata_win);
}
	
void in_cdata(WINDOW *in_cdata_win, WINDOW *in_cdata_subwin,
struct cdata *cdata, char xdg_path[], enum mode mode)
{
	FIELD *cdata_field[FIELDMAX + 1];
	FORM *cdata_form;
	int starty = 0, startx = 13;
	
	mvwin(in_cdata_win, (LINES - CWINY) / 2, (COLS - CWINX) / 2);
	wresize(in_cdata_win, CWINY, CWINX);
	
	curs_set(1);
	
	keypad(in_cdata_win, TRUE);	
	
	cdata_field[CITY] = new_field(1, 25, starty, startx, 0, 0);
	set_field_back(cdata_field[CITY], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(cdata_field[CITY], O_STATIC);
	field_opts_off(cdata_field[CITY], O_AUTOSKIP);
	starty += 2;
	
	cdata_field[YEAR] = new_field(1, 6, starty, startx, 0, 0);
	set_field_back(cdata_field[YEAR], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	set_field_type(cdata_field[YEAR], TYPE_INTEGER, 0, -12998, 16799);
	field_opts_off(cdata_field[YEAR], O_AUTOSKIP);
	starty += 2;
	
	cdata_field[MONTH] = new_field(1, 3, starty, startx, 0, 0);
	set_field_back(cdata_field[MONTH], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	set_field_type(cdata_field[MONTH], TYPE_INTEGER, 0, 1, 12);
	field_opts_off(cdata_field[MONTH], O_AUTOSKIP);
	starty += 2;
	
	cdata_field[DAY] = new_field(1, 3, starty, startx, 0, 0);
	set_field_back(cdata_field[DAY], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	set_field_type(cdata_field[DAY], TYPE_INTEGER, 0, 1, 31);
	field_opts_off(cdata_field[DAY], O_AUTOSKIP);
	starty += 2;
	
	cdata_field[HOUR] = new_field(1, 3, starty, startx, 0, 0);
	set_field_back(cdata_field[HOUR], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	set_field_type(cdata_field[HOUR], TYPE_INTEGER, 0, 0, 24);
	field_opts_off(cdata_field[HOUR], O_AUTOSKIP);
	starty+= 2;
	
	cdata_field[MINUTE] = new_field(1, 3, starty, startx, 0, 0);
	set_field_back(cdata_field[MINUTE], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	set_field_type(cdata_field[MINUTE], TYPE_INTEGER, 0, 0, 59);
	field_opts_off(cdata_field[MINUTE], O_AUTOSKIP);
	starty+= 2;
	
	cdata_field[SECOND] = new_field(1, 3, starty, startx, 0, 0);
	set_field_back(cdata_field[SECOND], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	set_field_type(cdata_field[SECOND], TYPE_INTEGER, 0, 0, 59);
	field_opts_off(cdata_field[SECOND], O_AUTOSKIP);
	starty+= 2;
	
	cdata_field[AMPM] = new_field(1, 3, starty, startx, 0, 0);
	set_field_back(cdata_field[AMPM], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	set_field_type(cdata_field[AMPM], TYPE_ALPHA, 0);
	field_opts_off(cdata_field[AMPM], O_AUTOSKIP);
	starty+= 2;
	
	cdata_field[TIMEZONE] = new_field(1, 30, starty, startx, 0, 0);
	set_field_back(cdata_field[TIMEZONE], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(cdata_field[TIMEZONE], O_STATIC);
	field_opts_off(cdata_field[TIMEZONE], O_AUTOSKIP);
	starty += 2;
	
	cdata_field[LATITUDE] = new_field(1, 11, starty, startx, 0, 0);
	set_field_back(cdata_field[LATITUDE], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	set_field_type(cdata_field[LATITUDE], TYPE_NUMERIC, 5, -90.0, 90.0);
	field_opts_off(cdata_field[LATITUDE], O_AUTOSKIP);
	starty+= 2;
	
	cdata_field[LONGITUDE] = new_field(1, 11, starty, startx, 0, 0);
	set_field_back(cdata_field[LONGITUDE], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	set_field_type(cdata_field[LONGITUDE], TYPE_NUMERIC, 5, -180.0, 180.0);
	field_opts_off(cdata_field[LONGITUDE], O_AUTOSKIP);
	
	cdata_field[FIELDMAX] = NULL;

	cdata_form = new_form(cdata_field);
	set_form_win(cdata_form, in_cdata_win);
	set_form_sub(cdata_form, in_cdata_subwin);
	
	post_form(cdata_form);
	
	set_current_field(cdata_form, cdata_field[CITY]);
	
	field_label(in_cdata_win);
	pos_form_cursor(cdata_form);
	
	int cdata_entry = 0, cancel = 0, ch = 0;
	while(!cdata_entry && (ch = wgetch(in_cdata_win)))
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
						
					case 'w':
						validate_fields(cdata_field, cdata_form, cdata, xdg_path);
						save_chart(cdata, xdg_path);
						mode = NORMAL;
						break;
						
					case 'e':
						load_chart(cdata, xdg_path);
						mode = NORMAL;
						cdata_entry = 1;
						break;
						
					case KEY_F(1):
						clear_fields(cdata_field, cdata_form);
						break;
						
					case 9: // tab
						setfield_localtime(cdata_field, cdata);
						break;
						
					case '\n':
						cdata_entry = 1;
						break;
						
					case 'q':
						cancel = 1;
						cdata_entry = 1;
						break;
				}
				break;
				
			case INSERT:
				switch (ch)
				{
					 case '\n':
						form_driver(cdata_form, REQ_VALIDATION);
						field_to_member(cdata, xdg_path, cdata_form, cdata_field);
						form_driver(cdata_form, REQ_NEXT_FIELD);
					
						field_label(in_cdata_win);
						
						form_driver(cdata_form, REQ_END_LINE);
						break;
						
					case KEY_DOWN: case ';':
						form_driver(cdata_form, REQ_NEXT_FIELD);
						form_driver(cdata_form, REQ_END_LINE);
						break;
						
					case KEY_UP: case '\'':
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
					
					case KEY_F(1):
						clear_fields(cdata_field, cdata_form);
						break;
						
					case 9: // tab
						setfield_localtime(cdata_field, cdata);
						break;
						
					case 27: // esc
						mode = NORMAL;
						break;
						
					case '\\':
						cdata_entry = 1;
						break;	
						
					default:
						form_driver(cdata_form, ch);
						break;
				}
				break;
		}
		wrefresh(in_cdata_win);
	}
	if (ch != 'e' && cancel != 1)
		validate_fields(cdata_field, cdata_form, cdata, xdg_path);

	unpost_form(cdata_form);
	werase(in_cdata_win);
	wrefresh(in_cdata_win);
	free_form(cdata_form);
	
	for (int i = CITY; i < FIELDMAX; ++i)
		free_field(cdata_field[i]);
	delwin(in_cdata_win);
}
