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
#include "init.h"

#define VERSION 0.74.3

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

	const char *moon[] = {"new", "crescent", "quarter", "gibbous", "full",
	"2nd gibbous", "2nd quarter", "2nd crescent"};
	
	enum mode mode = INSERT;
	
	char xdg_path[MAXBUF] = {0};
	
	int cur_chart = 1;
	int right_trig = 1, left_trig = 1;
		
	double cusp[CHARTMAX][13] = {0};
	double sign_cusp[CHARTMAX][13] = {0};
	
	double luna_eclipse[CHARTMAX][EMAX] = {0};
	double sol_eclipse[CHARTMAX][EMAX] = {0};
	
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
	
	struct pxx **pxx = calloc(CHARTMAX, sizeof(*pxx));
	if (!pxx)
		ERR_EXIT("main pxx");
	for (int i = 0; i < CHARTMAX; ++i)
	{
		pxx[i] = calloc(1, sizeof(*pxx[i]));
		if (!pxx[i])
			ERR_EXIT("ERR: pxx[i] calloc");
	}
		
	double *planet[SPXXMAX];
	planet_init(planet, cur_chart, pxx);
	
	struct cdata **cdata = calloc(CHARTMAX, sizeof(*cdata));
	if (!cdata)
		ERR_EXIT("main Location calloc");
	for (int i = 0; i < CHARTMAX; ++i)
	{
		cdata[i] = calloc(1, sizeof(*cdata[i]));
		if (!cdata[i])
			ERR_EXIT("ERR: cdata[i] calloc");
		cdata[i]->city = calloc(1, MAXBUF);
		if (!cdata[i]->city)
			ERR_EXIT("ERR: main cdata->city malloc");
		cdata[i]->state = calloc(1, MAXBUF);
		if (!cdata[i]->state)
			ERR_EXIT("ERR: main cdata->state malloc");
		cdata[i]->country = calloc(1, MAXBUF);
		if (!cdata[i]->country)
			ERR_EXIT("ERR: main cdata->country malloc");
		cdata[i]->timezone = calloc(1, MAXBUF);
		if (!cdata[i]->timezone)
			ERR_EXIT("ERR: main cdata->timezone malloc");
	}

	struct io **io = calloc(CHARTMAX, sizeof(*io));
	if (!io)
		ERR_EXIT("mai io calloc");
	for (int i = 0; i < CHARTMAX; ++i)
	{
		io[i] = calloc(1, sizeof(*io[i]));
		if (!io[i])
			ERR_EXIT("ERR: io[i] calloc");
		io[i]->filepath = calloc(1, MAXBUF);
		if (!io[i]->filepath)
			ERR_EXIT("main io->filepath malloc");
		io[i]->filename = calloc(1, MAXBUF);
		if (!io[i]->filename)
			ERR_EXIT("main io->filename malloc");
	}
	
	xdg_check(xdg_path, "ephe");
	
	if (strlen(xdg_path) > 255)
		ERR_EXIT("XDG_DATA_HOME path too long");
	swe_set_ephe_path(xdg_path);
	
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
	
	for (int i = 1; i < CHARTMAX; ++i)
	{
		set_localtime(cdata[i]);
		config_parse(cdata[i], xdg_path);
		calc_init(planet, sol_eclipse[i]);
	}
	
	new_chart(NEW_CHART_MAIN());
	doupdate();
	
	int main_done = 0;
	while (!main_done)
	{
		int chart_done = 0, ch = 0;
		while(!chart_done && !main_done &&
		(ch = wgetch(main_win)))
		{
			if (isdigit(ch))
			{
				cur_chart = ch - '0';
				if (cur_chart >= CHARTMAX || cur_chart <= 0)
					cur_chart = 10;
					
				calc_init(planet, sol_eclipse[cur_chart]);
				planet_init(planet, cur_chart, pxx);
				new_chart(NEW_CHART_MAIN());
				doupdate();
			}
			
			switch(ch)
			{
				case '\n':
					animate_chart(NEW_CHART_MAIN());
					doupdate();
					break;
				case 9: // tab
					calc_init(planet, sol_eclipse[cur_chart]);
					realtime_chart(NEW_CHART_MAIN());
					doupdate();
					break;
				case 'r':
					calc_init(planet, sol_eclipse[cur_chart]);
					new_chart(NEW_CHART_MAIN());
					doupdate();
					break;
				case 'q':
					main_done = 1;
					chart_done = 1;
					break;
				case 'd':
					if (cdata[cur_chart]->tm_isdst == 0)
						cdata[cur_chart]->tm_isdst = YDST;
					else if (cdata[cur_chart]->tm_isdst >= 1)
						cdata[cur_chart]->tm_isdst = NDST;
					new_chart(NEW_CHART_MAIN());
					doupdate();
					break;
				case 'i':
					mode = INSERT;
					in_cdata(in_cdata_win, in_cdata_subwin,
					io[cur_chart], cdata[cur_chart], xdg_path, mode);
			
					free(io[cur_chart]->filename);
					io[cur_chart]->filename = calloc(1, MAXBUF);
					if (!io[cur_chart]->filename)
						ERR_EXIT("case i io->filename calloc");
		
					calc_init(planet, sol_eclipse[cur_chart]);
					new_chart(NEW_CHART_MAIN());
					doupdate();
					break;
				case 'w':
					save_chart(cdata[cur_chart], io[cur_chart], xdg_path);
					new_chart(NEW_CHART_MAIN());
					doupdate();
					break;
				case 'e':
					load_chart(cdata[cur_chart], io[cur_chart], xdg_path);
					calc_init(planet, sol_eclipse[cur_chart]);
					new_chart(NEW_CHART_MAIN());
					doupdate();
					break;
				case 's':
					calc_init(planet, sol_eclipse[cur_chart]);
					solar_return(NEW_CHART_MAIN());
					break;
				case 'p':
					if (!left_trig)
					{
						left_table(left_win, planet, zodiac, pxx[cur_chart], cdata[cur_chart],
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
						right_table(right_win, luna_eclipse[cur_chart], sol_eclipse[cur_chart],
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
						right_table(right_win, luna_eclipse[cur_chart], sol_eclipse[cur_chart],
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
						left_table(left_win, planet, zodiac, pxx[cur_chart], cdata[cur_chart],
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
	
	for (int i = 0; i < CHARTMAX; ++i)
	{
		free(cdata[i]->state);
		free(cdata[i]->city);
		free(cdata[i]->country);
		free(cdata[i]->timezone);
		free(cdata[i]);
		free(pxx[i]);
		free(io[i]->filepath);
		free(io[i]->filename);
		free(io[i]);
	}
	
	free(cdata);
	free(pxx);
	free(io);
	free(zxx);
	
	return 0;
} 
