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

void ibirth_data()
{
	Bdata *bdata = malloc(sizeof(Bdata) * 4);
	
	char buff[256];
	
	printw("year?\n");
	getnstr(buff, 4);
	bdata->iyar = *buff;
	printw("month?\n");
	getnstr(buff, 2);
	bdata->imon = *buff;
	printw("day?\n");
	getnstr(buff, 2);
	bdata->iday = *buff;
	printw("city? (begin search)\n");
	getstr(buff);
	main_search(buff);
	free (bdata);
	clear();
}

int main()
{
	int i, c;
	int iret, iflag, ipl;
	double xx[6];
	char serr[AS_MAXCH];
	char spname[AS_MAXCH];
	Bdata bdata = {0};
	
	initscr();
	raw();
	swe_set_ephe_path("/home/plum/Builds/swisseph/ephe");
	ibirth_data();
	
	double jul_day_UT = swe_julday(bdata.iyar, bdata.imon, bdata.iday, bdata.dhour, SE_GREG_CAL);
	
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

