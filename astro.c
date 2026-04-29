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

#include <stdio.h>
#include <time.h>
#include <swephexp.h>
#include <ncurses.h>
#include <math.h>
#include <form.h>
#include <panel.h>
#include "astro.h"
#include "city-search.c"

#define CMAX 10

void fieldbuffer_trim(FIELD *current, char *buffer)
{
	//remove trailing spaces in ncurses fieldbuffer
	int i = 0;
	
	//set i to number of field collumns
	field_info(current, NULL, NULL, NULL, &i, NULL, NULL);
	
	while(i >= 0 && (buffer[i] == ' '))
		--i;
	if (i >= 0)
	{
		++i;
		buffer[i] = '\0';
	}
}

void field_to_member
(WINDOW *cdata_form_win, struct tm *cdata, Location *loc, 
FORM *cdata_form, FIELD *cdata_field[])
{
	FIELD *current = current_field(cdata_form);
	char *buffer = field_buffer(current, 0);
	int index = field_index(current);
	
	switch(index)
	{
		case 0:
			fieldbuffer_trim(current, buffer);
			main_search(cdata_field, buffer);
			//redraws field underline
			unpost_form(cdata_form);
			touchwin(cdata_form_win);
			post_form(cdata_form);
			wrefresh(cdata_form_win);
			doupdate();
			break;
		case 1:
			cdata->tm_year = atoi(buffer) - 1900;
			break;
		case 2:
			cdata->tm_mon = atoi(buffer) - 1;
			break;
		case 3: 
			cdata->tm_mday = atoi(buffer);
			break;
		case 4:
			fieldbuffer_trim(current, buffer);
			if (setenv("TZ", buffer, 1) != 0)
			{
				perror("TZ setenv");
				ERR_EXIT;
			}
			tzset();
			break;
		case 5:
			cdata->tm_hour = atoi(buffer);
			break;
		case 6:
			cdata->tm_min = atoi(buffer) - 1;
			break;
		case 7:
			loc->dlat = atof(buffer);
			break;
		case 8:
			loc->dlon = atof(buffer);
			break;
	}
}

void field_label(WINDOW *cdata_form_win, size_t i, int starty, int startx)
{
	const char *c_labels[] = {
		"city search:",
		"year:",
		"month:",
		"day:",
		"timezone:",
		"hour:",
		"minute:",
		"lat.",
		"long.",
		NULL
	};
	
	for (i = 0, starty = 4; i < 9; ++i, starty+= 2)
			mvwprintw(cdata_form_win, starty, startx - 12, "%s", c_labels[i]);
	wrefresh(cdata_form_win);
}
	
void ichart_data(struct tm *cdata, Location *loc)
{

	WINDOW *cdata_form_win;
	FIELD *cdata_field[CMAX];
	FORM *cdata_form;
	int ch;
	int starty, startx;
	size_t i = 0;
	
	int maxy, maxx;
	getmaxyx(stdscr, maxy, maxx);

	cbreak();
	noecho();
	
	cdata_form_win = newwin(maxy - 2, maxx - 2, 0, 0);
	keypad(cdata_form_win, TRUE);
	clearok(cdata_form_win, TRUE);
	wclear(cdata_form_win);
	wrefresh(cdata_form_win);
	
	starty = 4;
	startx = 18;
	
	// city search
	cdata_field[0] = new_field(1, 25, starty, startx, 0, 0);
	set_field_back(cdata_field[0], A_UNDERLINE);
	field_opts_off(cdata_field[0], O_STATIC);
	field_opts_off(cdata_field[0], O_AUTOSKIP);
	starty += 2;
	// year
	cdata_field[1] = new_field(1, 6, starty, startx, 0, 0);
	set_field_back(cdata_field[1], A_UNDERLINE);
	field_opts_off(cdata_field[1], O_AUTOSKIP);
	starty += 2;
	// month
	cdata_field[2] = new_field(1, 3, starty, startx, 0, 0);
	set_field_back(cdata_field[2], A_UNDERLINE);
	field_opts_off(cdata_field[2], O_AUTOSKIP);
	starty += 2;
	// day
	cdata_field[3] = new_field(1, 3, starty, startx, 0, 0);
	set_field_back(cdata_field[3], A_UNDERLINE);
	field_opts_off(cdata_field[3], O_AUTOSKIP);
	starty += 2;
	// timezone
	cdata_field[4] = new_field(1, 25, starty, startx, 0, 0);
	set_field_back(cdata_field[4], A_UNDERLINE);
	field_opts_off(cdata_field[4], O_STATIC);
	field_opts_off(cdata_field[4], O_AUTOSKIP);
	starty += 2;
	// hour
	cdata_field[5] = new_field(1, 3, starty, startx, 0, 0);
	set_field_back(cdata_field[5], A_UNDERLINE);
	field_opts_off(cdata_field[5], O_AUTOSKIP);
	starty+= 2;
	// minute
	cdata_field[6] = new_field(1, 3, starty, startx, 0, 0);
	set_field_back(cdata_field[6], A_UNDERLINE);
	field_opts_off(cdata_field[6], O_AUTOSKIP);
	starty+= 2;
	// lat. 
	cdata_field[7] = new_field(1, 11, starty, startx, 0, 0);
	set_field_back(cdata_field[7], A_UNDERLINE);
	field_opts_off(cdata_field[7], O_AUTOSKIP);
	starty+= 2;
	// long.
	cdata_field[8] = new_field(1, 11, starty, startx, 0, 0);
	set_field_back(cdata_field[8], A_UNDERLINE);
	field_opts_off(cdata_field[8], O_AUTOSKIP);
	
	cdata_field[9] = NULL;

	cdata_form = new_form(cdata_field);
	set_form_win(cdata_form, cdata_form_win);
	set_form_sub(cdata_form,
	derwin(cdata_form_win, maxy - 6, maxx - 6, 0, 0));
	
	touchwin(cdata_form_win);
	post_form(cdata_form);
	wrefresh(cdata_form_win);
	
	set_current_field(cdata_form, cdata_field[0]);
	
	wrefresh(cdata_form_win);
	field_label(cdata_form_win, i, starty, startx);
	pos_form_cursor(cdata_form);
	
	while((ch = wgetch(cdata_form_win)) != KEY_F(1))
	{
		switch(ch)
		{	
			case KEY_DOWN: case '\n':
				form_driver(cdata_form, REQ_VALIDATION);
				field_to_member(cdata_form_win, cdata, loc,
				cdata_form, cdata_field);
				form_driver(cdata_form, REQ_NEXT_FIELD);
				
				field_label(cdata_form_win, i, starty, startx);
				
				form_driver(cdata_form, REQ_END_LINE);
				break;
			case KEY_UP:
				form_driver(cdata_form, REQ_PREV_FIELD);
				form_driver(cdata_form, REQ_END_LINE);
				break;
			case KEY_BACKSPACE:
				form_driver(cdata_form, REQ_DEL_PREV);
				break;
			default:
				form_driver(cdata_form, ch);
				break;
		}
		wrefresh(cdata_form_win);
	}
	//validates every field, in case user didnt hit enter
	for (i = 1; i < 9; i++)
	{
		set_current_field(cdata_form, cdata_field[i]);
		form_driver(cdata_form, REQ_VALIDATION);
		field_to_member(cdata_form_win, cdata, loc, 
		cdata_form, cdata_field);
	}
	
	unpost_form(cdata_form);
	wclear(cdata_form_win);
	touchwin(cdata_form_win);
	wrefresh(cdata_form_win);
	free_form(cdata_form);
	
	for (i = 0; i < 7; ++i)
	{
		free_field(cdata_field[i]);
	}
	delwin(cdata_form_win);
}
void check_dst(struct tm *orig)
{
	char cmd[256] = {0};
	char buffer[256] = {0};
	char *tz_name = getenv("TZ");
	FILE *fp;
	
	//calls GNU coreutil date
	snprintf(cmd, sizeof(cmd), "TZ=%s date -d '%d-%d-%d %d:%d' '+%%Z'", 
	tz_name, orig->tm_year, orig->tm_mon, orig->tm_mday,
	orig->tm_hour, orig->tm_min);
	
	fp = popen(cmd, "r");
	if (!fp)
	{
		perror("date command fail");
		ERR_EXIT;
	}
	
	fgets(buffer, sizeof(buffer), fp);
	buffer[strcspn(buffer, "\n")] = 0;
	pclose(fp);
	
	orig->tm_isdst = 
	(tzname[1] && strcmp(buffer, tzname[1]) == 0) ? 1 : 0;
}

void chart_timeset(struct tm *cdata, Location *loc)
{
	struct tm *orig = cdata;
	check_dst(orig);
	
	//copy correct isdst and hour before mktime
	//mktime "corrects" it to system defaults, which can be wrong
	int isdst = orig->tm_isdst;
	int tm_hour = orig->tm_hour;
	int tm_min = orig->tm_min;
	
	time_t tret = mktime(orig);
	localtime_r(&tret, orig);
	
	orig->tm_isdst = isdst;
	orig->tm_hour = tm_hour;
	orig->tm_min = tm_min;

	long utc_sec = orig->tm_gmtoff;
	
	//get utc offset in seconds, reverse, and display in hours
	double utc_offset = (double)-utc_sec / 3600;
	
	//convert inputted minutes to decimal
	double min = (double)orig->tm_min / 60;
	
	//add inputted hour, utc offset, and minutes to decimal
	//swe_julday uses 24 hour UTC.
	double dhour = (double)(orig->tm_hour + utc_offset) + min;
	
	if(dhour > 23.999999)
	{
		dhour -= 23.999999;
		++orig->tm_mday;
	}
	if(dhour < 0)
	{
		dhour += 23.999999;
		--orig->tm_mday;
	}
	
	loc->dhour = dhour; 
	*cdata = *orig;
}

void reset_struct(struct tm *cdata)
{
	cdata->tm_min += 1;
	cdata->tm_mon += 1;
	cdata->tm_year += 1900;
}

void draw_circle(WINDOW *main_win,
int maxy, int maxx, int radius, chtype ch)
{
	int center_x = maxx / 2;
	int center_y = maxy / 2;
	
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

void planet_pos(WINDOW *main_win, int i, int maxy, int maxx,
int radius, double angle, double asc, char *pl_sym[])
{
	int center_x = (maxx / 2);
	int center_y = (maxy / 2);
	
	double rad = (angle - asc) * 3.15159 / 180.0;
	
	int x = center_x - (int)(radius * cos(rad));
	int y = center_y + (int)(radius * sin(rad) * 0.5);
	
	int offsetx = 0;
	for (int j = 0; j < 7; j++)
	{
		if (i == j)
		continue;
	double anglediff = fabs(angle);
	if (anglediff > 180)
		anglediff = 360 - anglediff;
	if (anglediff < 4)
		offsetx += 3;
	}
	
	char buffer[56];
	snprintf(buffer, sizeof(buffer), "%d", (int)angle % 30);
	mvwaddstr(main_win, y - 1, x, buffer);
	
	mvwaddstr(main_win, y, x + offsetx, pl_sym[i]);
}

void ascmc_pos(WINDOW *main_win, int i, int maxy, int maxx,
int radius, double angle, double asc, char *ascmc_sym[])
{
	int center_x = (maxx / 2);
	int center_y = (maxy / 2);
	
	double rad = (angle - asc) * 3.15159 / 180.0;
	
	int x = center_x - (int)(radius * cos(rad));
	int y = center_y + (int)(radius * sin(rad) * 0.5);
	
	mvwaddstr(main_win, y, x, ascmc_sym[i]);
	char buffer[56];
	snprintf(buffer, sizeof(buffer), "%d", (int)angle % 30);
	mvwaddstr(main_win, y - 1, x, buffer);
}

void zo_pos(WINDOW *main_win, int i, int maxy, int maxx,
int radius, double angle, double asc, char *zo_sym[])
{
	int center_x = (maxx / 2);
	int center_y = (maxy / 2);
	
	double rad = (angle - asc) * 3.15159 / 180.0;
	
	int x = center_x - (int)(radius * cos(rad));
	int y = center_y + (int)(radius * sin(rad) * 0.5);
	
	mvwaddstr(main_win, y, x, zo_sym[i]);
}

void draw_house(WINDOW *main_win, int maxy, int maxx, 
int radius, double angle, chtype ch)
{
	double rad = angle * 3.15159 / 180.0;
	
	int center_x = maxx / 2;
	int center_y = maxy / 2;
	
	int edge_x = center_x - (int)(radius * cos(rad));
	int edge_y = center_y + (int)(radius * sin(rad) * 0.5);
	
	int half_x = center_x - (int)((radius / 2) * cos(rad));
	int half_y = center_y + (int)((radius / 2)  * sin(rad) * 0.5);
	
	int dx = edge_x - half_x;
	int dy = edge_y - half_y;
	
	int distance = (int)sqrt(dx * dx + dy * dy);
	
	for(int i = 0; i <= distance; i++)
	{
		int x = half_x + (dx * i) / distance;
		int y = half_y + (dy * i) / distance;
		mvwaddch(main_win, y, x, ch);
	}
}

int main()
{
	WINDOW *main_win;
	int iret, iflag, ipl, i, c, done = 0;
	double xx[6];
	char serr[AS_MAXCH];
	char spname[AS_MAXCH];
	double cusps[13], ascmc[10]; //houses, asc, mc
	int ihsy = 'W'; // house system
	struct tm *cdata = calloc(1, sizeof(struct tm));
	if (!cdata)
	{
		perror("Cdata calloc");
		ERR_EXIT;
	}
	Location *loc = calloc(1, sizeof(Location));
	if (!loc)
	{
		perror("main Location calloc");
		ERR_EXIT;
	}
	P_deg *p_deg = calloc(1, sizeof(P_deg));
	if (!p_deg)
	{
		perror("P_deg calloc");
		ERR_EXIT;
	}
	//to fill each member of P_deg with its planets degree in later loops
	double *p_deg_members[] = {
	&p_deg->dsun, &p_deg->dmoon,
	&p_deg->dmerc, &p_deg->dven,
	&p_deg->dmars, &p_deg->djup,
	&p_deg->dsat};
	
	int maxy, maxx;
	
	initscr();
	getmaxyx(stdscr, maxy, maxx);
	cbreak();
	swe_set_ephe_path("/home/plum/Builds/swisseph/ephe");

	main_win = newwin(maxy, maxx, 0, 0);
	keypad(main_win, TRUE);
	wrefresh(main_win);
	
	while (!done)
	{
		curs_set(1);
		ichart_data(cdata, loc); // ----
		chart_timeset(cdata, loc); // goes before swe_julday
		reset_struct(cdata); // ----
		double jul_day_UT = swe_julday(cdata->tm_year, cdata->tm_mon, 
		cdata->tm_mday, loc->dhour, SE_GREG_CAL);

		refresh();
		
		wrefresh(main_win);
		
		iflag = SEFLG_SWIEPH | SEFLG_SPEED;
		for (ipl = SE_SUN, i = 0; ipl <= SE_SATURN; ipl++, i++)
		{
			swe_get_planet_name(ipl, spname);
			spname[7] = '\0';
			iret = swe_calc_ut(jul_day_UT, ipl, iflag, xx, serr);
			if (iret < 0) 
			{
				fprintf(stderr, "%s", serr);
				ERR_EXIT;
				exit(EXIT_FAILURE);
			}
			*p_deg_members[i] = xx[0];
			
		}
		
		iret = swe_houses_ex(jul_day_UT, 0, loc->dlat, loc->dlon,
		ihsy, cusps, ascmc);
		if (iret < 0)
		{
			fprintf(stderr, "%s", serr);
			ERR_EXIT;
			exit(EXIT_FAILURE);
		}
		
		curs_set(0);
		int radius = ((maxx / 2 < maxy) ? maxx / 2 : maxy) - 5;
		draw_circle(main_win, maxy, maxx, radius, '.');
		draw_circle(main_win, maxy, maxx, (radius / 2) - 1, '.');
		for (i = 0; i < 13; ++i)
		{
			draw_house(main_win, maxy, maxx, radius,
			cusps[i], '.');
		}
		char *pl_sym[] = {"Su", "Mo", "Me", "V", "Ma", "J", "Sa"};
		for (i = 0; i < 7; ++i)
		{
			planet_pos(main_win, i, maxy, maxx,
			radius - 9, *p_deg_members[i], cusps[1],
			pl_sym);
		}
		char *ascmc_sym[] = {"as", "mc"};
		for (i = 0; i < 2; ++i)
		{
			ascmc_pos(main_win, i, maxy, maxx,
			radius - 5, ascmc[i], cusps[1], ascmc_sym);
		}
			
		int asc_sign = (int)(ascmc[0] / 30);
		
		char *zo_sym[] = {NULL, "aries", "taurus", "gemini", "cancer",
		"leo", "virgo", "libra", "scorpio", "sagitarius",
		"capricorn", "aquarius", "pisces"};
		for (i = 1; i < 13; ++i)
		{
			int sign_display = ((i - 1 + asc_sign) % 12) + 1;
			zo_pos(main_win, sign_display, maxy, maxx,
			radius + 3, cusps[i] + 45, ascmc[0], zo_sym);
		}
	
		wrefresh(main_win);
		
		int chart_done = 0;
		while(!chart_done && !done)
		{
			c = wgetch(main_win);
			switch(c)
			{
				case 'q':
					done = 1;
					chart_done = 1;
					break;
				case 'i':
					wclear(main_win);
					chart_done = 1;
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
	free(loc);
	free(p_deg);
	return 0;
}

