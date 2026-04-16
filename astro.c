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
#include "astro.h"
#include "city-search.c"

int ya_input(char s[])
{
	int i, c;
	for (i = 0; i < AS_MAXCH; ++i)
		s[i] = 0;
	for (i = 0; i < AS_MAXCH && (c = getch()) != '\n'; ++i)
		s[i] = c;
	s[i] = '\0';
	return atoi(s);
}	

void char_input(char s[])
{
	int i, c;
	for (i = 0; i < AS_MAXCH; ++i)
		s[i] = 0;
	for (i = 0; i < AS_MAXCH && (c = getch()) != '\n'; ++i)
		s[i] = c;
	s[i] = '\0';
}

int main()
{
	int i, c;
	char input_arr[AS_MAXCH];
	int iyar;
	int imon;
	int iday;
	double dhour = 23.122;
	int iret, iflag, ipl;
	double xx[6];
	char serr[AS_MAXCH];
	char spname[AS_MAXCH];
	
	initscr();
	raw();
	swe_set_ephe_path("/home/plum/Builds/swisseph/ephe");
	
	printw("year?\n");
	iyar = ya_input(input_arr);
	printw("month?\n");
	imon = ya_input(input_arr);
	printw("day?\n");
	iday = ya_input(input_arr);
	printw("city? (begin search)\n");
	char_input(input_arr);
	main_search(input_arr);
	
	double jul_day_UT = swe_julday(iyar, imon, iday, dhour, SE_GREG_CAL);
	
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

