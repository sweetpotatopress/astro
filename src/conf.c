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

#include <form.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include "swephexp.h"
#include "astro.h"
#include "io.h"
#include "ui.h"

#define C_ASP 0
#define C_TIMEZONE 1
#define C_LATITUDE 2
#define C_LONGITUDE 3
#define C_NULL 4
#define C_MAX 5

static void ftb(struct ui *ui, FIELD **field, char **buffer)
{
	snprintf(buffer[C_ASP], MAXBUF, "%d", ui->aspect_trig);
	
	for (int i = C_TIMEZONE; i < C_NULL; ++i)
	{
		const char *v = field_buffer(field[i], 0);
		
		snprintf(buffer[i], MAXBUF, "%s", v);
		size_t len = strlen(buffer[i]);
		while (len > 0 && (buffer[i][len-1] == ' ' || buffer[i][len-1] == '\n'))
			buffer[i][--len] = '\0';
	}
}

static void btf(FIELD **field, char **buffer)
{
	for (int i = C_TIMEZONE; i < C_NULL; ++i)
		set_field_buffer(field[i], 0, buffer[i]);
}
		
static void set_current(FIELD **field, struct cdata *cdata)
{
	char xdg_path[MAXBUF] = {0};
	xdg_check(xdg_path, "config");
	
	char buf[MAXBUF] = {0};
	
	set_field_buffer(field[C_TIMEZONE], 0, cdata->timezone);
	
	snprintf(buf, sizeof buf, "%f", cdata->dlat);
	set_field_buffer(field[C_LATITUDE], 0, buf);
	
	snprintf(buf, sizeof buf, "%f", cdata->dlon);
	set_field_buffer(field[C_LONGITUDE], 0, buf);
}
	
static void config_parse(char **buffer)
{
	char xdg_path[MAXBUF] = {0};
	xdg_check(xdg_path, "config");
	
	FILE *fp = fopen(xdg_path, "r");
	if (fp == NULL)
		return;
		
	for (int i = C_ASP; i < C_NULL; ++i)
	{
		if (fgets(buffer[i], MAXBUF, fp) == NULL)
			break;
		
		buffer[i][strcspn(buffer[i], "\n")] = '\0';
	}
	fclose(fp);
}

static void config_set(struct cdata *cdata, struct ui *ui, char **buffer)
{
	if (setenv("TZ", buffer[C_TIMEZONE], 1) != 0)
		ERR_EXIT("ERR: setenv conf.c");
	tzset();
	memcpy(cdata->timezone, buffer[C_TIMEZONE], strlen(buffer[C_TIMEZONE]) + 1);
	
	char *endptr = NULL;
	double dret;
	long lret;
	errno = 0;
	
	lret = strtol(buffer[C_ASP], &endptr, 10);
	if (errno != ERANGE)
		ui->aspect_trig = (int)lret;
	errno = 0;
	
	dret = strtod(buffer[C_LATITUDE], &endptr);
	if (errno != ERANGE)
		cdata->dlat = dret;
	errno = 0;
	
	dret = strtod(buffer[C_LONGITUDE], &endptr);
	if (errno != ERANGE)
		cdata->dlon = dret;
	errno = 0;
}

void config_init(struct cdata *cdata, struct ui *ui)
{
	char **buffer = ecalloc(C_NULL, sizeof *buffer);
	for (int i = C_ASP; i < C_NULL; ++i)
		buffer[i] = ecalloc(MAXBUF ,sizeof *buffer[i]);
	config_parse(buffer);
	config_set(cdata, ui, buffer);
	for (int i = C_ASP; i < C_NULL; ++i)
		free(buffer[i]);
	free(buffer);
}

static void config_write(struct cdata *cdata, struct ui *ui)
{
	char xdg_path[MAXBUF] = {0};
	xdg_check(xdg_path, "config");
	
	FILE *ifp = fopen(xdg_path, "w");
	if (!ifp)
		ERR_EXIT("ERR: set_current ifp");
		
	fprintf(ifp, "%d\n%s\n%f\n%f\n",
		ui->aspect_trig,
		cdata->timezone,
		cdata->dlat,
		cdata->dlon);
		
	fclose(ifp);
}

static void field_label(struct ui *ui)
{
	const char *label[] = {
		"aspect",
		"timezone",
		"latitude",
		"longitude",
		NULL };
		
	int y = 3, x = 1;
	for (size_t i = C_ASP; i < C_NULL; ++i, y+=2)
		mvwprintw(ui->config_win, y, x, "%s", label[i]);
}

static void button_toggle(FIELD **field, int idx, int *trig)
{
	if (*trig)
	{
		set_field_buffer(field[idx], 0, "off");
		set_field_back(field[idx], COLOR_PAIR (FIRE) | A_REVERSE);
	}
	else
	{
		set_field_buffer(field[idx], 0, "on ");
		set_field_back(field[idx], COLOR_PAIR (EARTH) | A_REVERSE);
	}
}

void config_menu(struct cdata *cdata, struct ui *ui)
{
	FIELD *field[C_MAX] = { NULL };
	FORM *form = NULL;
	int sy = 0, sx = 13;
	
	char **buffer = ecalloc(C_NULL, sizeof *buffer);
	for (int i = C_ASP; i < C_NULL; ++i)
		buffer[i] = ecalloc(MAXBUF ,sizeof *buffer[i]);
	
	ui->config_win = newwin(CWINY, CWINX, CWIN_Y, CWIN_X);
	ui->config_subwin = derwin(ui->config_win, CWINY-4, CWINX-2, 3, 1);
	
	char *header = "conf - [tab]fill [space]toggle [enter]set -";
	
	mvwin(ui->config_win, (LINES - CWINY), (COLS - CWINX));
	wresize(ui->config_win, CWINY, CWINX);
	mvwprintw(ui->config_win, 1, 1, "%s", header);
	mvwhline(ui->config_win, 2, 1, ACS_HLINE, CWINX - 2);
	box(ui->config_win, 0, 0);
	
	curs_set(1);
	keypad(ui->config_win, TRUE);
	
	field[C_ASP] = new_field(1, 3, sy, sx, 0, 0);
	button_toggle(field, C_ASP, &ui->aspect_trig);
	field_opts_off(field[C_ASP], O_EDIT);
	sy += 2;
	
	field[C_TIMEZONE] = new_field(1, 30, sy, sx, 0, 0);
	set_field_back(field[C_TIMEZONE], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(field[C_TIMEZONE], O_STATIC);
	field_opts_off(field[C_TIMEZONE], O_AUTOSKIP);
	sy += 2;
	
	field[C_LATITUDE] = new_field(1, 11, sy, sx, 0, 0);
	set_field_back(field[C_LATITUDE], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	set_field_type(field[C_LATITUDE], TYPE_NUMERIC, 5, -90.0, 90.0);
	field_opts_off(field[C_LATITUDE], O_AUTOSKIP);
	sy+= 2;
	
	field[C_LONGITUDE] = new_field(1, 11, sy, sx, 0, 0);
	set_field_back(field[C_LONGITUDE], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	set_field_type(field[C_LONGITUDE], TYPE_NUMERIC, 5, -180.0, 180.0);
	field_opts_off(field[C_LONGITUDE], O_AUTOSKIP);
	
	field[C_NULL] = NULL;

	form = new_form(field);
	set_form_win(form, ui->config_win);
	set_form_sub(form, ui->config_subwin);
	
	post_form(form);
    field_label(ui);
    
	config_parse(buffer);
	btf(field, buffer);
	
	set_current_field(form, field[C_ASP]);
	pos_form_cursor(form);

	wrefresh(ui->config_win);
	
	int done = 0, ch = 0;
	while (!done && (ch = wgetch(ui->config_win)))
	{
		switch (ch)
		{
			case ' ':
				if (current_field(form) == field[C_ASP])
				{
					ui->aspect_trig = !ui->aspect_trig;
					button_toggle(field, C_ASP, &ui->aspect_trig);
				}
				break;
			case 9:
				set_current(field, cdata);
				pos_form_cursor(form);
				break;
				
			case KEY_DOWN: case ';':
				form_driver(form, REQ_NEXT_FIELD);
				form_driver(form, REQ_END_LINE);
				break;
				
			case KEY_UP: case '\'':
				form_driver(form, REQ_PREV_FIELD);
				form_driver(form, REQ_END_LINE);
				break;
				
			case KEY_LEFT:
				form_driver(form, REQ_LEFT_CHAR);
				break;
				
			case KEY_RIGHT:
				form_driver(form, REQ_RIGHT_CHAR);
				break;
				
			case KEY_BACKSPACE:
				form_driver(form, REQ_DEL_PREV);
				break;
			
			case '\\': case '[': case ']': case '\n':
				done = 1;
				break;	
				
			case 27:
				goto cleanup;
				break;
				
			default:
				form_driver(form, ch);
				break;
		}
		pos_form_cursor(form);
		box(ui->config_win, 0, 0);
		wrefresh(ui->config_win);
	}
	for(int i = C_TIMEZONE; i < C_NULL; ++i)
	{
		set_current_field(form, field[i]);
		form_driver(form, REQ_VALIDATION);
	}
	ftb(ui, field, buffer);
	config_set(cdata, ui, buffer);
	config_write(cdata, ui);
	
	cleanup:
	unpost_form(form);
	free_form(form);
	
	for (int i = C_ASP; i < C_NULL; ++i)
	{
		free_field(field[i]);
		free(buffer[i]);
	}
	delwin(ui->config_win);
	free(buffer);
}
