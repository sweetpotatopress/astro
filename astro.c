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
#include <swephexp.h>
#include <ncurses.h>
#include <math.h>
#include <form.h>
#include <panel.h>
#include "astro.h"
#include "city-search.c"

#define CMAX 7

void field_to_member(Cdata *cdata, FORM *cdata_form)
{
	FIELD *current = current_field(cdata_form);
	char *buffer = field_buffer(current, 0);
	int index = field_index(current);
	
	switch(index)
	{
		case 0:
			printw("%s", buffer);
			break;
		case 1:
			cdata->iyar = atoi(buffer);
			break;
		case 2:
			cdata->imon = atoi(buffer);
			break;
		case 3: 
			cdata->iday = atoi(buffer);
			break;
		case 4:
			cdata->dhour = atof(buffer);
			break;
		case 5:
			cdata->dlon = atof(buffer);
			break;
		case 6:
			cdata->dlat = atof(buffer);
			break;
	}
}

void ichart_data(Cdata *cdata)
{

	FIELD *cdata_field[CMAX];
	FORM *cdata_form;
	int ch;
	int starty, startx;
	
	const char *c_labels[] = {
		"city search:",
		"year:",
		"month:",
		"day:",
		"hour:",
		"long.",
		"lat.",
		NULL
	};
	
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	
	starty = 4;
	startx = 18;
	
	for (int i = 0; i < CMAX; ++i)
	{
		cdata_field[i] = new_field(1, 10, starty, startx, 0, 0);
		set_field_back(cdata_field[i], A_UNDERLINE);
		starty += 2;
	}
	cdata_field[CMAX] = NULL;
	
	cdata_form = new_form(cdata_field);
	post_form(cdata_form);
	refresh();
	
	for (int i = 0, starty = 4; i < CMAX; ++i, starty+= 2)
		mvprintw(starty, startx - 12, "%s", c_labels[i]);
	refresh();
	
	while((ch = getch()) != KEY_F(1))
	{
		switch(ch)
		{	
			case KEY_DOWN: case '\n':
				form_driver(cdata_form, REQ_VALIDATION);
				field_to_member(cdata, cdata_form);
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
	for (int i = 0; i < 7; ++i)
	{
		free_field(cdata_field[i]);
	}
	endwin();
}

int main()
{
	int iret, iflag, ipl;
	double xx[6];
	char serr[AS_MAXCH];
	char spname[AS_MAXCH];
	Cdata *cdata = calloc(1, sizeof(Cdata));
	if (!cdata)
	{
		perror("Cdata calloc");
		ERR_EXIT;
		exit(EXIT_FAILURE);
	}
	WINDOW *main;
	PANEL *main_panel;
	int maxy, maxx;
	getmaxyx(stdscr, maxy, maxx);
	
	initscr();
	raw();
	swe_set_ephe_path("/home/plum/Builds/swisseph/ephe");
	main = newwin(maxx, maxy, 0, 0);
	box(main, 0, 0);
	main_panel = new_panel(main);
	update_panels();
	doupdate();
	getch();
	ichart_data(cdata);
	printw("%d, %d, %d, %f, %f, %f", cdata->iyar, cdata->imon, cdata->iday, cdata->dhour, cdata->dlon, cdata->dlat);
	refresh();
	
	double jul_day_UT = swe_julday(cdata->iyar, cdata->imon,
	cdata->iday, cdata->dhour, SE_GREG_CAL);
	
	printw("\njulian day:%lf\n", jul_day_UT);
	iflag = SEFLG_SWIEPH;
	for (ipl = SE_SUN; ipl <= SE_TRUE_NODE; ipl++)
	{
		swe_get_planet_name(ipl, spname);
		spname[7] = '\0';
		printw("\n%s\t", spname);
		iret = swe_calc_ut(jul_day_UT, ipl, iflag, xx, serr);
		printw("%10.6lf\t%9.6lf\t%9.6lf\t%9.6lf\n", xx[0], xx[1], xx[2], xx[3]);
	}
	printw("\n%d", iret);
	refresh();
	getch();
	endwin();
	swe_close();
	free(cdata);
	return 0;
}

