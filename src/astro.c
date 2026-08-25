/* Copyright (C) 2026 yam lynn
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License
as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTIBILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. if not, see <https://www.gnu.org/licenses/> */

#include <ncurses.h>
#include <panel.h>
#include "swephexp.h"
#include "astro.h"
#include "table.h"
#include "io.h"
#include "draw.h"
#include "anim.h"
#include "chronos.h"
#include "indat.h"

#define VERSION 0.73.3

int main()
{
	// sun, moon, mercury, venus, mars, jupiter,
	// saturn, uranus, neptune, pluto, south, north node
	const char *pl_sym[] = {"(o)", "(()", "(-o<)",
	"(~:o)", "(o->)", "(\\+)", "(h)", "(\\*/)", "(?)",
	"(P)", "(u)", "(^)"};

	// the swiss ephemeris skips 0 for some reason
	const char *zo_sym[] = {0, "ari", "tau", "gem", "can",
	"leo", "vir", "lib", "sco", "sag",
	"cap", "aqu", "pis"};

	// condensed from valens 11 phases to the 'main 8' inspired by rudhyar 
	const char *moon[] = {"new", "crescent", "quarter", "gibbous", "full",
	"2nd gibbous", "2nd quarter", "2nd crescent"};
	
	enum mode mode = INSERT;
	
	struct cdata *cdata = calloc(1, sizeof(*cdata));
	if (!cdata)
		ERR_EXIT("main Location calloc");
	cdata->city = calloc(1, MAXBUF);
	if (!cdata->city)
		ERR_EXIT("ERR: main cdata->city malloc");
	cdata->state = calloc(1, MAXBUF);
	if (!cdata->state)
		ERR_EXIT("ERR: main cdata->state malloc");
	cdata->country = calloc(1, MAXBUF);
	if (!cdata->country)
		ERR_EXIT("ERR: main cdata->country malloc");
	cdata->timezone = calloc(1, MAXBUF);
	if (!cdata->timezone)
		ERR_EXIT("ERR: main cdata->timezone malloc");
		
	struct zxx *zxx = calloc(1, sizeof(*zxx));
	if (!zxx)
		ERR_EXIT("main zxx calloc");
		
	int *zodiac[] = {
		0,
		zxx->iari, zxx->itau,
		zxx->igem, zxx->ican,
		zxx->ileo, zxx->ivir,
		zxx->ilib, zxx->isco,
		zxx->isag, zxx->icap,
		zxx->iaqu, zxx->ipis};
		
	zxx_init(zodiac); // fills essential dignities
		
	struct pxx *pxx = calloc(1, sizeof(*pxx));
	if (!pxx)
		ERR_EXIT("main pxx");
		
	double *planet[] = {
		pxx->dsun, pxx->dmoon,
		pxx->dmerc, pxx->dven,
		pxx->dmars, pxx->djup,
		pxx->dsat, pxx->dura,
		pxx->dnep, pxx->dplu,
		pxx->dmnod, pxx->dtnod,
		pxx->dasc, pxx->dmc,
		pxx->ddsc, pxx->dic,
		pxx->dfor, pxx->dspir};

	struct io *io = calloc(1, sizeof(*io));
	if (!io)
		ERR_EXIT("mai io calloc");
	io->filepath = calloc(1, MAXBUF);
	if (!io->filepath)
		ERR_EXIT("main io->filepath malloc");
	io->filename = calloc(1, MAXBUF);
	if (!io->filename)
		ERR_EXIT("main io->filename malloc");
		
	double cusp[13];
	double sign_cusp[13];
	
	double luna_eclipse[EMAX] = {0};
	double sol_eclipse[EMAX] = {0};
	
	const char *home_dir = getenv("HOME");
	if (!home_dir)
		ERR_EXIT("HOME environment not set");
		
	char fn_buff[MAXBUF] = {0};
		
	const char *xdg_data = getenv("XDG_DATA_HOME");
	if (!xdg_data)
		snprintf(fn_buff, MAXBUF, 
		"%s/.local/share/astro/ephe", home_dir);
	else
		snprintf(fn_buff, MAXBUF, 
		"%s/astro/ephe", xdg_data);
	
	swe_set_ephe_path(fn_buff);
	
	initscr();
	set_escdelay(25);
	start_color();
	cbreak();
	noecho();

	init_pair(M_COLOR, COLOR_WHITE,  COLOR_BLACK);
	init_pair(FIRE,    COLOR_RED,    COLOR_BLACK);
	init_pair(EARTH,   COLOR_GREEN,  COLOR_BLACK);
	init_pair(AIR,     COLOR_YELLOW, COLOR_BLACK);
	init_pair(WATER,   COLOR_BLUE,   COLOR_BLACK);

	PANEL *main_panel;
	WINDOW *main_win = newwin(LINES, COLS, 0, 0);
	main_panel = new_panel(main_win);
	hide_panel(main_panel);
	
	wbkgdset(main_win, COLOR_PAIR(M_COLOR));
	
	WINDOW *in_cdata_win = newwin(CWINY, CWINX, CWIN_Y, CWIN_X);
	WINDOW *in_cdata_subwin = derwin(in_cdata_win, CWINY-2, CWINX-2, 1, 1);
	
	wbkgdset(in_cdata_win, COLOR_PAIR(M_COLOR));
	
	PANEL *left_panel;
	WINDOW *left_win = newwin(LWINY, LWINX, LWIN_Y, LWIN_X);
	left_panel = new_panel(left_win);
	hide_panel(left_panel);
	
	wbkgdset(left_win, COLOR_PAIR(M_COLOR));
	
	PANEL *right_panel;
	WINDOW *right_win = newwin(RWINY, RWINX, RWIN_Y, RWIN_X);
	right_panel = new_panel(right_win);
	hide_panel(right_panel);
	
	wbkgdset(right_win, COLOR_PAIR(M_COLOR));
	
	keypad(main_win, TRUE);
	keypad(stdscr, TRUE);

	werase(stdscr);
	wrefresh(stdscr);
	show_panel(main_panel);
	
	set_localtime(cdata);
	config_parse(cdata);
	
	int right_trig = 1, left_trig = 1;
	new_chart(NEW_CHART_MAIN());
	doupdate();
	
	int main_done = 0;
	while (!main_done)
	{
		int chart_done = 0, ch = 0;
		while(!chart_done && !main_done &&
		(ch = wgetch(main_win)))
		{
			switch(ch)
			{
				case '\n':
					animate_chart(NEW_CHART_MAIN());
					doupdate();
					break;
				case 9: // tab
					ECLIPSE_INIT();
					realtime_chart(NEW_CHART_MAIN());
					doupdate();
					break;
				case 'r':
					ECLIPSE_INIT();
					new_chart(NEW_CHART_MAIN());
					doupdate();
					break;
				case 'q':
					main_done = 1;
					chart_done = 1;
					break;
				case 'd':
					if (cdata->tm_isdst == 0)
						cdata->tm_isdst = YDST;
					else if (cdata->tm_isdst >= 1)
						cdata->tm_isdst = NDST;
					new_chart(NEW_CHART_MAIN());
					doupdate();
					break;
				case 'i':
					mode = INSERT;
					in_cdata(in_cdata_win, in_cdata_subwin,
					io, cdata, mode);
			
					free(io->filename);
					io->filename = calloc(1, MAXBUF);
					if (!io->filename)
						ERR_EXIT("case i io->filename calloc");
		
					ECLIPSE_INIT();
					new_chart(NEW_CHART_MAIN());
					doupdate();
					break;
				case 'w':
					save_chart(cdata, io);
					new_chart(NEW_CHART_MAIN());
					doupdate();
					break;
				case 'e':
					load_chart(cdata, io);
					ECLIPSE_INIT();
					new_chart(NEW_CHART_MAIN());
					doupdate();
					break;
				case 's':
					ECLIPSE_INIT();
					solar_return(NEW_CHART_MAIN());
					break;
				case 'p':
					if (!left_trig)
					{
						left_table(left_win, planet, zodiac, pxx,
						pl_sym, zo_sym, moon);
						show_panel(left_panel);
						left_trig = 1;
					}
					else
					{
						hide_panel(left_panel);
						clear();
						refresh();
						left_trig = 0;
					}
					
					if (right_trig > 0)
					{
						right_table(right_win, luna_eclipse, sol_eclipse,
						planet, zodiac, zo_sym, pl_sym);
						show_panel(right_panel);
					}
					
					touchwin(main_win);
					wnoutrefresh(main_win);
					update_panels();
					doupdate();
					break;
				case 'o':
					if (!right_trig)
					{
						right_table(right_win, luna_eclipse, sol_eclipse,
						planet, zodiac, zo_sym, pl_sym);
						show_panel(right_panel);
						right_trig = 1;
					}
					else
					{
						hide_panel(right_panel);
						clear();
						refresh();
						right_trig = 0;
					}	
					
					if (left_trig > 0)
					{
						left_table(left_win, planet, zodiac, pxx,
						pl_sym, zo_sym, moon);
						show_panel(left_panel);
					}
					
					touchwin(main_win);
					wnoutrefresh(main_win);
					update_panels();
					doupdate();
					break;
				default:
					break;
			}
		}
	}
	delwin(main_win);
	endwin();
	swe_close();
	
	free(cdata->state);
	free(cdata->city);
	free(cdata->country);
	free(cdata->timezone);
	free(cdata);
	free(zxx);
	free(pxx);
	free(io->filepath);
	free(io->filename);
	free(io);
	
	return 0;
} 
