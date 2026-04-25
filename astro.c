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
	int i = 0;
	
	field_info(current, NULL, NULL, NULL, &i, NULL, NULL);
	
	while(i >= 0 && (buffer[i] == ' '))
		--i;
	if (i >= 0)
	{
		++i;
		buffer[i] = '\0';
	}
}

void field_to_member(struct tm *cdata, Location *loc, FORM *cdata_form)
{
	FIELD *current = current_field(cdata_form);
	char *buffer = field_buffer(current, 0);
	int index = field_index(current);
	
	switch(index)
	{
		case 0:
			fieldbuffer_trim(current, buffer);
			main_search(buffer);
			break;
		case 1:
			cdata->tm_year = atoi(buffer);
			break;
		case 2:
			cdata->tm_mon = atoi(buffer);
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
			printw("\t %s", getenv("TZ"));
			printw("--tzname0 = %s tzname1 = %s", tzname[0], tzname[1]);
			break;
		case 5:
			cdata->tm_hour = atoi(buffer);
			break;
		case 6:
			cdata->tm_min = atoi(buffer);
			break;
		case 7:
			loc->dlat = atof(buffer);
			break;
		case 8:
			loc->dlon = atof(buffer);
			break;
	}
}

void chart_timeset(struct tm *cdata, Location *loc)
{
	time_t tret = mktime(cdata);
	if (tret == (time_t)-1)
	{
		fprintf(stderr, "mktime fail");
		ERR_EXIT;
	}
	printw(" tret: %ld ", tret);
	struct tm tmp = {0};
	struct tm orig = *cdata;
	localtime_r(&tret, &tmp);
	
	long utc_sec = tmp.tm_gmtoff;
	
	long utc_off = utc_sec / 3600;
	double min = orig.tm_min / 60;
	printw(" cmin %f ", min);
	double dhour = (double)(orig.tm_hour + utc_off) + min;
	if(dhour > 23.999999)
	{
		double offset = dhour - 23.999999;
		dhour = offset;
		++cdata->tm_mday;
	}
	if(dhour < 0)
	{
		double offset = dhour + 23.999999;
		dhour = offset;
		--cdata->tm_mday;
	}
	
	loc->dhour = dhour; 
	*cdata = orig;
}

void ichart_data(struct tm *cdata, Location *loc)
{

	FIELD *cdata_field[CMAX];
	FORM *cdata_form;
	int ch;
	int starty, startx;
	size_t i;
	
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
	
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	
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
	cdata_field[2] = new_field(1, 2, starty, startx, 0, 0);
	set_field_back(cdata_field[2], A_UNDERLINE);
	field_opts_off(cdata_field[2], O_AUTOSKIP);
	starty += 2;
	// day
	cdata_field[3] = new_field(1, 2, starty, startx, 0, 0);
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
	cdata_field[5] = new_field(1, 2, starty, startx, 0, 0);
	set_field_back(cdata_field[5], A_UNDERLINE);
	field_opts_off(cdata_field[5], O_AUTOSKIP);
	starty+= 2;
	// minute
	cdata_field[6] = new_field(1, 2, starty, startx, 0, 0);
	set_field_back(cdata_field[6], A_UNDERLINE);
	field_opts_off(cdata_field[6], O_AUTOSKIP);
	starty+= 2;
	// lat. 
	cdata_field[7] = new_field(1, 8, starty, startx, 0, 0);
	set_field_back(cdata_field[7], A_UNDERLINE);
	field_opts_off(cdata_field[7], O_AUTOSKIP);
	starty+= 2;
	// long.
	cdata_field[8] = new_field(1, 8, starty, startx, 0, 0);
	set_field_back(cdata_field[8], A_UNDERLINE);
	field_opts_off(cdata_field[8], O_AUTOSKIP);
	
	cdata_field[9] = NULL;
	
	cdata_form = new_form(cdata_field);
	post_form(cdata_form);
	refresh();
	
	for (i = 0, starty = 4; i < 9; ++i, starty+= 2)
		mvprintw(starty, startx - 12, "%s", c_labels[i]);
	refresh();
	
	while((ch = getch()) != KEY_F(1))
	{
		switch(ch)
		{	
			case KEY_DOWN: case '\n':
				form_driver(cdata_form, REQ_VALIDATION);
				field_to_member(cdata, loc,  cdata_form);
				form_driver(cdata_form, REQ_NEXT_FIELD);
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
		pos_form_cursor(cdata_form);
		refresh();
	}
	unpost_form(cdata_form);
	free_form(cdata_form);
	for (i = 0; i < 7; ++i)
	{
		free_field(cdata_field[i]);
	}
	endwin();
}
void draw_circle(int maxy, int maxx, int radius, chtype ch)
{
	int center_x = maxx /2;
	int center_y = maxy / 2;
	
	for (int angle = 0; angle < 360; angle +=5)
	{
		double rad = angle * 3.14159 / 180.0;
		int x = center_x + (int)(radius * cos(rad));
		int y = center_y + (int)(radius * sin(rad) * 0.5);
		
		mvaddch(y, x, ch);
	}
}	

int main()
{
	int iret, iflag, ipl, i;
	double xx[6];
	char serr[AS_MAXCH];
	char spname[AS_MAXCH];
	double cusps[13], ascmc[10];
	int ihsy = 'W';
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
	
	WINDOW *main;
	//PANEL *main_panel;
	int maxy, maxx;
	
	initscr();
	getmaxyx(stdscr, maxy, maxx);
	raw();
	swe_set_ephe_path("/home/plum/Builds/swisseph/ephe");

	main = newwin(maxy, maxx, 0, 0);
	box(main, 0, 0);
	//main_panel = new_panel(main);
	//update_panels();
	doupdate();
	getch();
	
	ichart_data(cdata, loc); //this has to go before swe_julday
	chart_timeset(cdata, loc);
	double jul_day_UT = swe_julday(cdata->tm_year, cdata->tm_mon, 
	cdata->tm_mday, loc->dhour, SE_GREG_CAL);

	printw(" %s ", cdata->tm_zone);
	printw("%d, %d, %d, %d, %d, %f, %f, dhour:%f", 
	cdata->tm_year, cdata->tm_mon, cdata->tm_mday,
	cdata->tm_hour, cdata->tm_min, loc->dlon, loc->dlat, loc->dhour);
	refresh();
	
	draw_circle(maxy, maxx, (maxy / 2) + 5, '*');
	//printw("\njulian day:%lf\n", jul_day_UT);
	
	iflag = SEFLG_SWIEPH | SEFLG_SPEED;
	for (ipl = SE_SUN, i = 0; ipl <= SE_SATURN; ipl++, i++)
	{
		swe_get_planet_name(ipl, spname);
		spname[7] = '\0';
		printw("\n%s\t", spname);
		iret = swe_calc_ut(jul_day_UT, ipl, iflag, xx, serr);
		if (iret < 0) 
		{
			fprintf(stderr, "%s", serr);
			ERR_EXIT;
			exit(EXIT_FAILURE);
		}
		*p_deg_members[i] = xx[0];
		printw("%10.6lf\t%9.6lf\t%9.6lf\t%9.6lf\n", xx[0], xx[1], xx[2], xx[3]);
		
	}
	//printw("%f p_deg", p_deg->dsun);
	//printw("%f sat", p_deg->dsat);
	
	iret = swe_houses_ex(jul_day_UT, 0, loc->dlat, loc->dlon,
	ihsy, cusps, ascmc);
	if (iret < 0)
	{
		fprintf(stderr, "%s", serr);
		ERR_EXIT;
		exit(EXIT_FAILURE);
	}
	//printw("asc %10.6lf", ascmc[0]);
	//for (int i = 1; i <= 12; i++)
	//{
	//	printw("cusp %2d  %10.6lf", i, cusps[i]);
	//}
	
	refresh();
	getch();
	endwin();
	swe_close();
	free(cdata);
	free(loc);
	free(p_deg);
	return 0;
}

