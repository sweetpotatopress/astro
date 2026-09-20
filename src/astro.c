// Copyright (C) 2026 yam lynn
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTIBILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License for more details.

// You should have received a copy of the GNU Affero General Public License
// along with this program. if not, see <https://www.gnu.org/licenses/>

#include <unistd.h>
#include <time.h>
#include <ncurses.h>
#include <panel.h>
#include "swephexp.h"
#include "astro.h"
#include "table.h"
#include "io.h"
#include "anim.h"
#include "chronos.h"
#include "indat.h"
#include "init.h"

#define VERSION "0.75.7"

void *ecalloc(size_t n, size_t size)
{
	void *p;
	if (!(p = calloc(n, size)))
		ERR_EXIT("\ncalloc: out of memory\n");
	return p;
}

void *erealloc(void *p, size_t size)
{
	if (!(p = realloc(p, size)))
		ERR_EXIT("\nrealloc: out of memory\n");
	return p;
}

static void iana_check(void)
{
	const char *iana_path[] = { 
	"/usr/share/zoneinfo/America/New_York",
	"/usr/share/lib/zoneinfo/America/New_York",
	"/usr/local/share/zoneinfo/America/New_York",
	"/usr/local/etc/zoneinfo/America/New_York"}; 
	int c = 1; 
	for (int i = 0; i < 4; ++i) 
		if (access(iana_path[i], F_OK) == 0) 
			c = 0; 
	if (c)
		ERR_EXIT("ERR: no IANA timezone data installed");
}

int main(int argc, char *argv[])
{
	int opt;
	while ((opt = getopt(argc, argv, "v")) != -1)
	{
		switch (opt)
		{
			case 'v':
				puts(VERSION);
				return EXIT_SUCCESS;
			default:
				return EXIT_FAILURE;
		}
	}
	
	iana_check();
	
	char xdg_path[MAXBUF] = {0};
	xdg_check(xdg_path, "ephe");
	
	if (strlen(xdg_path) > 255)
		ERR_EXIT("XDG_DATA_HOME path too long");
	swe_set_ephe_path(xdg_path);

	initscr();
	set_escdelay(25);
	start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	
	// alt + 1-0
	define_key("\0331", 1001);
	define_key("\0332", 1002);
	define_key("\0333", 1003);
	define_key("\0334", 1004);
	define_key("\0335", 1005);
	define_key("\0336", 1006);
	define_key("\0337", 1007);
	define_key("\0338", 1008);
	define_key("\0339", 1009);
	define_key("\0330", 1000);

	init_pair(M_COLOR, COLOR_WHITE,  COLOR_BLACK);
	init_pair(FIRE,    COLOR_RED,    COLOR_BLACK);
	init_pair(EARTH,   COLOR_GREEN,  COLOR_BLACK);
	init_pair(AIR,     COLOR_YELLOW, COLOR_BLACK);
	init_pair(WATER,   COLOR_BLUE,   COLOR_BLACK);

	struct cdata **cdata = ecalloc(CHARTMAX, sizeof(*cdata));
	
	for (int i = 0; i < CHARTMAX; ++i)
	{
		cdata[i] = ecalloc(1, sizeof(*cdata[i]));
		cdata[i]->city = ecalloc(1, MAXBUF);
		cdata[i]->state = ecalloc(1, MAXBUF);
		cdata[i]->country = ecalloc(1, MAXBUF);
		cdata[i]->timezone = ecalloc(1, MAXBUF);
		cdata[i]->chart_name = ecalloc(1, MAXBUF);
	}
	
	struct zxx *zxx = ecalloc(1, sizeof(*zxx));
		
	int *zodiac[] = {
		0,
		zxx->iari, zxx->itau,
		zxx->igem, zxx->ican,
		zxx->ileo, zxx->ivir,
		zxx->ilib, zxx->isco,
		zxx->isag, zxx->icap,
		zxx->iaqu, zxx->ipis};
		
	zxx_init(zodiac); // fills essential dignities

	struct ui *ui = ecalloc(1, sizeof(*ui));
	
	ui->sym = (struct ui_sym) {
		.pl_sym = { "(o)", "(()", "(-o<)",
		"(~:o)", "(o->)", "(\\+)", "(h)", "(\\*/)", "(?)",
		"(P)", "(u)", "(^)" },
	
		.zo_sym = { "err", "ari", "tau", "gem", "can",
		"leo", "vir", "lib", "sco", "sag",
		"cap", "aqu", "pis" },
	
		.moon = { "new", "crescent", "quarter", "gibbous", "full",
		"2nd gibbous", "2nd quarter", "2nd crescent" },
		
		.month = { "err", "jan", "feb", "mar", "apr", "may", "jun",
		"jul", "aug", "sep", "oct", "nov", "dec" }
	};
	
	ui->cc = 1;
	
	ui->left_trig = 1;
	ui->right_trig = 1;
	
	ui->main_win = newwin(LINES, COLS, 0, 0);
	ui->main_panel = new_panel(ui->main_win);

	hide_panel(ui->main_panel);
	
	wbkgdset(ui->main_win, COLOR_PAIR(M_COLOR));
	
	ui->indat_win = newwin(CWINY, CWINX, CWIN_Y, CWIN_X);
	ui->indat_subwin = derwin(ui->indat_win, CWINY-2, CWINX-2, 1, 1);
	
	wbkgdset(ui->indat_win, COLOR_PAIR(M_COLOR));
	
	ui->left_win = newwin(LWINY, LWINX, LWIN_Y, LWIN_X);
	ui->left_panel = new_panel(ui->left_win);
	hide_panel(ui->left_panel);
	
	wbkgdset(ui->left_win, COLOR_PAIR(M_COLOR));
	
	ui->right_win = newwin(RWINY, RWINX, RWIN_Y, RWIN_X);
	ui->right_panel = new_panel(ui->right_win);
	hide_panel(ui->right_panel);
	
	wbkgdset(ui->right_win, COLOR_PAIR(M_COLOR));
	
	struct pxx **pxx = ecalloc(CHARTMAX, sizeof(*pxx));
	
	for (int i = 0; i < CHARTMAX; ++i)
		pxx[i] = ecalloc(1, sizeof(*pxx[i]));
		
	double *planet[SPXXMAX];
	planet_init(planet, ui->cc, pxx);
	
	keypad(ui->main_win, TRUE);
	show_panel(ui->main_panel);
	
	for (int i = 1; i < CHARTMAX; ++i)
	{
		set_localtime(cdata[i]);
		config_parse(cdata[i], xdg_path);
		ecst_init(planet, cdata[i]->se);
	}
	
	wheel_init(ui->main_win, ui, 0, 0, 0);
	new_chart(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
	doupdate();
	
	int ch = 0;
	bool done = 0;
	while(!done && (ch = wgetch(ui->main_win)))
	{
		if (isdigit(ch))
		{
			ui->cc = ch - '0';
			if (ui->cc >= CHARTMAX || ui->cc <= 0)
				ui->cc = 10;
				
			set_localtime(cdata[TRANSIT]);
			ecst_init(planet, cdata[ui->cc]->se);
			planet_init(planet, ui->cc, pxx);
			new_chart(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
			doupdate();
		}
		if (ch >= 1000 && ch <= 1009)
		{
			int key = ch - 1000;
			if (key == 0)
				key = 10;
			synastry(cdata, pxx, ui, planet, zodiac, key);
			getch();
			planet_init(planet, ui->cc, pxx);
			ecst_init(planet, cdata[ui->cc]->se);
			new_chart(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
			doupdate();
		}
		
		switch(ch)
		{
			case '\n':
				animate_chart(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
				new_chart(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
				doupdate();
				break;
			case 9: // tab
				ecst_init(planet, cdata[ui->cc]->se);
				realtime_chart(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
				doupdate();
				break;
			case 'z':
				zodiacal_releasing(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
				new_chart(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
				doupdate();
				break;
			case 't':
				transit(cdata, pxx, ui, planet, zodiac);
				ecst_init(planet, cdata[ui->cc]->se);
				planet_init(planet, ui->cc, pxx);
				new_chart(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
				doupdate();
				break;
			case 'r':
				ecst_init(planet, cdata[ui->cc]->se);
				wheel_init(ui->main_win, ui, 0, 0, 0);
				new_chart(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
				doupdate();
				break;
			case 'R':
				cdata_clear(cdata[ui->cc]);
				ecst_init(planet, cdata[ui->cc]->se);
				planet_init(planet, ui->cc, pxx);
				config_parse(cdata[ui->cc], xdg_path);
				set_localtime(cdata[ui->cc]);
				wheel_init(ui->main_win, ui, 0, 0, 0);
				new_chart(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
				doupdate();
				break;
			case 'q':
				done = 1;
				break;
			case 'd':
				if (cdata[ui->cc]->isdst == NDST)
					cdata[ui->cc]->isdst = YDST;
				else if (cdata[ui->cc]->isdst >= YDST)
					cdata[ui->cc]->isdst = NDST;
				new_chart(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
				doupdate();
				break;
			case 'i':
				cdata_clear(cdata[ui->cc]);
				in_cdata(cdata[ui->cc], ui, xdg_path);
				
				ecst_init(planet, cdata[ui->cc]->se);
				new_chart(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
				doupdate();
				break;
			case 'w':
				save_chart(cdata[ui->cc], xdg_path);
				new_chart(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
				doupdate();
				break;
			case 'e':
				load_chart(cdata[ui->cc], xdg_path);
				ecst_init(planet, cdata[ui->cc]->se);
				new_chart(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
				doupdate();
				break;
			case 's':
				ecst_init(planet, cdata[ui->cc]->se);
				solar_return(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
				break;
			case 'p':
				if (!ui->left_trig)
				{
					left_table(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
					show_panel(ui->left_panel);
					ui->left_trig = 1;
				}
				else
				{
					hide_panel(ui->left_panel);
					clear();
					refresh();
					ui->left_trig = 0;
				}
				
				if (ui->right_trig > 0)
				{
					right_table(cdata[ui->cc], ui, planet, zodiac);
					show_panel(ui->right_panel);
				}
				
				touchwin(ui->main_win);
				wnoutrefresh(ui->main_win);
				update_panels();
				doupdate();
				break;
			case 'o':
				if (!ui->right_trig)
				{
					right_table(cdata[ui->cc], ui, planet, zodiac);
					show_panel(ui->right_panel);
					ui->right_trig = 1;
				}
				else
				{
					hide_panel(ui->right_panel);
					clear();
					refresh();
					ui->right_trig = 0;
				}	
				
				if (ui->left_trig > 0)
				{
					left_table(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
					show_panel(ui->left_panel);
				}
				
				touchwin(ui->main_win);
				wnoutrefresh(ui->main_win);
				update_panels();
				doupdate();
				break;
			default:
				break;
		}
	}
	del_panel(ui->right_panel);
	del_panel(ui->left_panel);
	del_panel(ui->main_panel);
	delwin(ui->main_win);
	delwin(ui->right_win);
	delwin(ui->left_win);
	endwin();
	swe_close();
	
	for (int i = 0; i < CHARTMAX; ++i)
	{
		free(cdata[i]->state);
		free(cdata[i]->city);
		free(cdata[i]->country);
		free(cdata[i]->timezone);
		free(cdata[i]->chart_name);
		free(cdata[i]);
		free(pxx[i]);
	}
	
	free(cdata);
	free(pxx);
	free(zxx);
	free(ui);
	
	return 0;
} 
