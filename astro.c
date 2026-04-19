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
#include "astro.h"
#include "city-search.c"

void chart_data()
{

	FIELD *cdata_field[7];
	FORM *cdata_form;
	int ch;
	int height, width, starty, startx;
	
	initscr();
	cbreak();
	keypad(stdscr, TRUE);
	
	height = 10, width = 30;
	starty = 4;
	startx = 18;
	
	for (int i = 0; i < 7; ++i, starty += 2)
	{
		cdata_field[i] = new_field(1, 10, starty, startx, 0, 0);
		cdata_field[7] = NULL;
		set_field_back(cdata_field[i], A_UNDERLINE);
	}
	
	cdata_form = new_form(cdata_field);
	post_form(cdata_form);
	refresh();
	
	while((ch = getch()) != KEY_F(1))
	{
		switch(ch)
		{	
			case KEY_DOWN:
				form_driver(cdata_form, REQ_NEXT_FIELD);
				form_driver(cdata_form, REQ_END_LINE);
				break;
			case KEY_UP:
				form_driver(cdata_form, REQ_NEXT_FIELD);
				form_driver(cdata_form, REQ_END_LINE);
			default:
				form_driver(cdata_form, ch);
				break;
		}
	}
	unpost_form(cdata_form);
	free_form(cdata_form);
	for (int i = 0; i < 7; ++i)
		free_field(cdata_field[i]);
	
	Cdata *cdata = malloc(sizeof(Cdata) * 4);
	
	char buff[256];
	
	endwin();
	free (cdata);
}

int main()
{
	int iret, iflag, ipl;
	double xx[6];
	char serr[AS_MAXCH];
	char spname[AS_MAXCH];
	Cdata cdata = {0};
	
	initscr();
	raw();
	swe_set_ephe_path("/home/plum/Builds/swisseph/ephe");
	chart_data();
	
	double jul_day_UT = swe_julday(cdata.iyar, cdata.imon, cdata.iday, cdata.dhour, SE_GREG_CAL);
	
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
	refresh();
	getch();
	endwin();
	swe_close();
	return 0;
}

