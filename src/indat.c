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
#include "ui.h"
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
	
	struct tm gettime;
		
	time_t now = time(NULL);
	localtime_r(&now, &gettime);
	
	if (gettime.tm_hour == 0)
		gettime.tm_hour = 12;
	
	if (gettime.tm_hour > 12)
	{
		gettime.tm_hour -= 12;
		set_field_buffer(cdata_field[AMPM], 0, "pm");
	}
	else
		set_field_buffer(cdata_field[AMPM], 0, "am");
	
	snprintf(buff, sizeof(buff), "%d", gettime.tm_year+1900);
	set_field_buffer(cdata_field[YEAR], 0, buff);
	
	snprintf(buff, sizeof(buff), "%d", gettime.tm_mon + 1);
	set_field_buffer(cdata_field[MONTH], 0, buff);
	
	snprintf(buff, sizeof(buff), "%d", gettime.tm_mday);
	set_field_buffer(cdata_field[DAY], 0, buff);
	
	snprintf(buff, sizeof(buff), "%d", gettime.tm_hour);
	set_field_buffer(cdata_field[HOUR], 0, buff);
	
	snprintf(buff, sizeof(buff), "%d", gettime.tm_min);
	set_field_buffer(cdata_field[MINUTE], 0, buff);
	
	snprintf(buff, sizeof(buff), "%d", gettime.tm_sec);
	set_field_buffer(cdata_field[SECOND], 0, buff);
}

static void field_to_member (struct cdata *cdata, struct ui *ui, char xdg_path[], FORM *cdata_form, FIELD *cdata_field[])
{
	char *endptr = NULL;
	long iret;
	double dret;
	errno = 0;
	
	FIELD *current = current_field(cdata_form);
	int index = field_index(current);
	
	char buffer[MAXBUF] = {0};
	char *f = field_buffer(current, 0);
	int len = 0;
	field_info(current, NULL, &len, NULL, NULL, NULL, NULL);
	
	if (len <= 0)
		buffer[0] = '\0';
	
	memcpy(buffer, f, (size_t)len);
	
	while(len > 0 && buffer[len - 1] == ' ')
		--len;
	buffer[len] = '\0';
	
	switch(index)
	{
		case CITY:
			city_search(cdata, ui, xdg_path, cdata_field, cdata_form, buffer);
			touchwin(ui->main_win);
			wnoutrefresh(ui->main_win);
			if (ui->left_trig > 0)
				top_panel(ui->left_panel);
			update_panels();
			doupdate();
			
			break;
			
		case YEAR:
			iret = strtol(buffer, &endptr, 10);
			if (errno != ERANGE)
				cdata->year = (int)iret;
			else
				cdata->year = 1970;
			break;
			
		case MONTH:
			iret = strtol(buffer, &endptr, 10);
			if (errno != ERANGE && iret != -1)
				cdata->mon = (int)iret;
			else
				cdata->mon = 1;
			break;
			
		case DAY: 
			iret = strtol(buffer, &endptr, 10);
			if (errno != ERANGE && iret != -1)
				cdata->mday = (int)iret;
			else
				cdata->mday = 1;
			break;
			
		case HOUR:
			iret = strtol(buffer, &endptr, 10);
			if (errno != ERANGE && iret != -1 && iret <= 12) 
				cdata->hour = (int)iret;
			else
				cdata->hour = 1;
			break;
			
		case MINUTE:
			iret = strtol(buffer, &endptr, 10);
			if (errno != ERANGE && iret != -1)
				cdata->min = (int)iret;
			else
				cdata->min = 1;
			break;
			
		case SECOND:
			iret = strtol(buffer, &endptr, 10);
			if (errno != ERANGE && iret != -1)
				cdata->sec = (int)iret;
			else
				cdata->sec = 1;
			break;
		
		case AMPM:
			if ((!strcasecmp(buffer, "p") || !strcasecmp(buffer, "pm"))
			&& cdata->hour != 12)
				cdata->hour += 12;
			if (cdata->hour >= 24)
				cdata->hour = 0;
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

static void field_label(struct ui *ui)
{
	const char *label[] = {
		"city search:",
		"year:",
		"month:",
		"day:",
		"hour:",
		"minute:",
		"second:",
		"am/pm:",
		" ",
		"timezone:",
		"latitude:",
		"longitude:",
		NULL };
		
	int y = 1, x = 1;
	for (size_t i = CITY; i < FIELDMAX; ++i, y += 2)
		mvwprintw(ui->indat_win, y, x, "%s", label[i]);
}

void in_cdata(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac, char xdg_path[])
{
	char tmp_city[MAXBUF] = {0};
	char tmp_state[MAXBUF] = {0};
	char tmp_country[MAXBUF] = {0};
	
	snprintf(tmp_city, sizeof tmp_city, "%s", cdata->city);
	snprintf(tmp_state, sizeof tmp_state, "%s", cdata->state);
	snprintf(tmp_country, sizeof tmp_country, "%s", cdata->country);
	
	FIELD *cdata_field[FIELDMAX + 1];
	FORM *cdata_form;
	int starty = 0, startx = 13;
	
	mvwin(ui->indat_win, (LINES - IWINY), (COLS - IWINX));
	wresize(ui->indat_win, IWINY, IWINX);
	ui_resize(cdata, pxx, ui, planet, zodiac, 0);
	box(ui->indat_win, 0, 0);
	
	curs_set(1);
	
	keypad(ui->indat_win, TRUE);	
	
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
	
	cdata_field[DRAW] = new_field(1, 8, starty, startx, 0, 0);
	set_field_buffer(cdata_field[DRAW], 0, "[ draw ]");
	field_opts_off(cdata_field[DRAW], O_EDIT);
	starty += 2;
	
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
	set_form_win(cdata_form, ui->indat_win);
	set_form_sub(cdata_form, ui->indat_subwin);
	
	post_form(cdata_form);
	
	set_current_field(cdata_form, cdata_field[CITY]);
	
	field_label(ui);
	wrefresh(ui->indat_win);
	pos_form_cursor(cdata_form);

	FIELD *current = NULL;
	int index = 0;
	int cdata_entry = 0, ch = 0;
	while(!cdata_entry && (ch = wgetch(ui->indat_win)))
	{
		current = current_field(cdata_form);
		index = field_index(current);
		
		switch (ch)
		{
			 case '\n':
				if (index == CITY)
				{
					form_driver(cdata_form, REQ_VALIDATION);
					field_to_member(cdata, ui, xdg_path, cdata_form, cdata_field);
				}
				if (index == DRAW)
				{
					memset(cdata->chart_name, 0, MAXBUF);
					cdata->isdst = -1;
					cdata_entry = 1;
					break;
				}
	
				form_driver(cdata_form, REQ_NEXT_FIELD);
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
				for (int i = 0; i < FIELDMAX; i++)
				{
					set_current_field(cdata_form, cdata_field[i]);
					form_driver(cdata_form, REQ_CLR_FIELD);
				}
				set_current_field(cdata_form, cdata_field[CITY]);
				break;
				
			case 9: // tab
				setfield_localtime(cdata_field, cdata);
				break;
				
			case '\\': case '[': case ']':
				memset(cdata->chart_name, 0, MAXBUF);
				cdata->isdst = -1;
				cdata_entry = 1;
				break;	
				
			case 27:
				unpost_form(cdata_form);
				free_form(cdata_form);
				for (int i = CITY; i < FIELDMAX; ++i)
					free_field(cdata_field[i]);
				delwin(ui->indat_win);
				snprintf(cdata->city, sizeof tmp_city, "%s", tmp_city);
				snprintf(cdata->state, sizeof tmp_state, "%s", tmp_state);
				snprintf(cdata->country, sizeof tmp_country, "%s", tmp_country);
				ui_resize(cdata, pxx, ui, planet, zodiac, 1);
				return;
				break;
				
			default:
				form_driver(cdata_form, ch);
				break;
		}
		current = current_field(cdata_form);
		index = field_index(current);
		
		if (index == DRAW)
		{
			wattron(ui->indat_win, A_REVERSE);
			mvwprintw(ui->indat_win, 17, 14, "[ draw ]");
			wattroff(ui->indat_win, A_REVERSE);
		}
		else
			mvwprintw(ui->indat_subwin, 16, 13, "[ draw ]");
			
		pos_form_cursor(cdata_form);
		box(ui->indat_win, 0, 0);
		wrefresh(ui->indat_win);
	}
	for (int i = 1; i < FIELDMAX; i++)
	{
		set_current_field(cdata_form, cdata_field[i]);
		form_driver(cdata_form, REQ_VALIDATION);
		field_to_member(cdata, ui, xdg_path, cdata_form, cdata_field);
	}

	unpost_form(cdata_form);
	werase(ui->indat_win);
	wrefresh(ui->indat_win);
	free_form(cdata_form);
	
	for (int i = CITY; i < FIELDMAX; ++i)
		free_field(cdata_field[i]);
	ui_resize(cdata, pxx, ui, planet, zodiac, 1);
	delwin(ui->indat_win);
}

