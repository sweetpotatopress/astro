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
#include "astro.h"
#include "io.h"
#include "draw.h"
#include "chronos.h"
#include "cdata.h"

#define VERSION 0.65

int main()
{
	// sun, moon, mercury, venus, mars, jupiter,
	// saturn, uranus, neptune, pluto, south, north node
	const char *pl_sym[] = {"(o)", "(()", "(-o<)",
	"(~:o)", "(o->)", "(\\+)", "(h)", "(\\*/)", "(?)",
	"(P)", "(u)", "(^)"};

	// 0 = NULL because the swiss ephemeris skips 0
	const char *zo_sym[] = {NULL, "ari", "tau", "gem", "can",
	"leo", "vir", "lib", "sco", "sag",
	"cap", "aqu", "pis"};

	const char *moon[] = {"new", "crescent", "1st quarter", "gibbous", "full",
	"dissem.", "last quarter", "balsamic"};
	
	enum mode mode = INSERT;
	
	struct cdata *cdata = calloc(1, sizeof(*cdata));
	if (!cdata)
		ERR_EXIT("main Location calloc");
	cdata->city = malloc(MAXBUF);
	if (!cdata->city)
		ERR_EXIT("ERR: main cdata->city malloc");
		
	struct zxx *zxx = calloc(1, sizeof(*zxx));
	if (!zxx)
		ERR_EXIT("main zxx calloc");
		
	int *z_arr[] = {
		0,
		zxx->iari, zxx->itau,
		zxx->igem, zxx->ican,
		zxx->ileo, zxx->ivir,
		zxx->ilib, zxx->isco,
		zxx->isag, zxx->icap,
		zxx->iaqu, zxx->ipis};
		
	zxx_init(z_arr); // fills essential dignities
		
	struct pxx *pxx = calloc(1, sizeof(*pxx));
	if (!pxx)
		ERR_EXIT("main pxx");
		
	double *p_arr[] = {
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
	io->filepath = malloc(MAXBUF);
	if (!io->filepath)
		ERR_EXIT("main io->filepath malloc");
	io->filename = malloc(256);
	if (!io->filename)
		ERR_EXIT("main io->filename malloc");
		
	char *citybuffer = malloc(MAXBUF);
	if (!citybuffer)
		ERR_EXIT("ERR: main citybuffer alloc fail");
	char *statebuffer = malloc(MAXBUF);
	if (!statebuffer)
		ERR_EXIT("ERR: main statebuffer alloc fail");
	char *countrybuffer = malloc(MAXBUF);
	if (!countrybuffer)
		ERR_EXIT("ERR: main countrybuffer alloc fail");
		
	double cusps[13];
	
	const char *home_dir = getenv("HOME");
	if (!home_dir)
		ERR_EXIT("HOME environment not set");
		
	char fn_buff[MAXBUF] = {0};
	snprintf(fn_buff, MAXBUF, 
	"%s/.local/share/astro/ephe", home_dir);
	
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
	
	PANEL *planet_panel;
	WINDOW *planet_win = newwin(PWINY, PWINX, PWIN_Y, PWIN_X);
	planet_panel = new_panel(planet_win);
	hide_panel(planet_panel);
	
	wbkgdset(planet_win, COLOR_PAIR(M_COLOR));
	
	PANEL *retro_panel;
	WINDOW *retro_win = newwin(RWINY, RWINX, RWIN_Y, RWIN_X);
	retro_panel = new_panel(retro_win);
	hide_panel(retro_panel);
	
	wbkgdset(retro_win, COLOR_PAIR(M_COLOR));
	
	keypad(main_win, TRUE);
	keypad(stdscr, TRUE);

	werase(stdscr);
	wrefresh(stdscr);
	show_panel(main_panel);
	
	set_localtime(cdata);
	pxx_init(cusps, p_arr, cdata, pxx);
	draw_chart(main_win, cusps, p_arr, z_arr, pxx, cdata,
	pl_sym, zo_sym);
	cur_chart_data(main_win, io, cdata);

	cdata->city = citybuffer;
	cdata->state = statebuffer;
	cdata->country = countrybuffer;
	
	planet_table(planet_win, p_arr, z_arr, pxx,
	pl_sym, zo_sym, moon);
	retro_table(retro_win, p_arr, pl_sym);
		
	show_panel(planet_panel);
	show_panel(retro_panel);
	
	touchwin(main_win);
	wnoutrefresh(main_win);
	update_panels();
	doupdate();
	
	int main_done = 0;
	while (!main_done)
	{
		static int retro_trig = 1, planet_trig = 1;
		
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
					realtime_chart(NEW_CHART_MAIN());
					doupdate();
					break;
				case 'r':
					new_chart(NEW_CHART_MAIN());
					doupdate();
					break;
				case 'q':
					main_done = 1;
					chart_done = 1;
					break;
				case 'i':
					mode = INSERT;
					in_cdata(in_cdata_win, in_cdata_subwin,
					io, cdata, mode,
					citybuffer, statebuffer, countrybuffer);
			
					free(io->filename);
					io->filename = NULL;
					
					new_chart(NEW_CHART_MAIN());
					doupdate();
					break;
				case 'w':
					save_chart(cdata, io,
					citybuffer, statebuffer, countrybuffer);
					doupdate();
					break;
				case 'e':
					load_chart(cdata, io,
					citybuffer, statebuffer, countrybuffer);
					new_chart(NEW_CHART_MAIN());
					doupdate();
					break;
				case 's':
					solar_return(NEW_CHART_MAIN());
					break;
				case 'p':
					if (!planet_trig)
					{
						planet_table(planet_win, p_arr, z_arr, pxx,
						pl_sym, zo_sym, moon);
						show_panel(planet_panel);
						planet_trig = 1;
					}
					else
					{
						hide_panel(planet_panel);
						clear();
						refresh();
						planet_trig = 0;
					}
					
					if (retro_trig > 0)
					{
						retro_table(retro_win, p_arr, pl_sym);
						show_panel(retro_panel);
					}
					
					touchwin(main_win);
					wnoutrefresh(main_win);
					update_panels();
					doupdate();
					break;
				case 'o':
					if (!retro_trig)
					{
						retro_table(retro_win, p_arr, pl_sym);
						show_panel(retro_panel);
						retro_trig = 1;
					}
					else
					{
						hide_panel(retro_panel);
						clear();
						refresh();
						retro_trig = 0;
					}	
					
					if (planet_trig > 0)
					{
						planet_table(planet_win, p_arr, z_arr, pxx,
						pl_sym, zo_sym, moon);
						show_panel(planet_panel);
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
	
	free(cdata);
	free(citybuffer);
	free(statebuffer);
	free(countrybuffer);
	free(pxx);
	
	return 0;
} 
