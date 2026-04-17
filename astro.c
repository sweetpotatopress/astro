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

void chart_data()
{
	WINDOW *cdata_win;
	static int startx, starty, width, height;
	static int x, y;
	x = y = 3;
	height = 10;
	width = 30;
	starty = (LINES - height) / 2;
	startx = (COLS - width) / 2;
	
	initscr();
	cbreak();
	
	cdata_win = newwin(height, width, starty, startx);
	refresh();
	Cdata *cdata = malloc(sizeof(Cdata) * 4);
	
	char buff[256];
	
	mvwprintw(cdata_win, y, x, "year ");
	wgetnstr(cdata_win,buff, 4);
	cdata->iyar = *buff;
	wclear(cdata_win);
	
	mvwprintw(cdata_win, y, x, "month ");
	wgetnstr(cdata_win, buff, 2);
	cdata->imon = *buff;
	wclear(cdata_win);
	
	mvwprintw(cdata_win, y, x, "day ");
	wgetnstr(cdata_win, buff, 2);
	cdata->iday = *buff;
	wclear(cdata_win);
	
	mvwprintw(cdata_win, y, x, "city ");
	wgetnstr(cdata_win, buff, sizeof(buff) -1);
	main_search(buff);
	
	wrefresh(cdata_win);
	wclear(cdata_win);
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

