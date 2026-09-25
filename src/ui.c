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

#include <ncurses.h>
#include <string.h>
#include "swephexp.h"
#include "astro.h"
#include "ui.h"
#include "init.h"
#include "chronos.h"
#include "draw.h"

void ui_resize(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac, bool x)
{
	if (!x)
	{
		ui->old_l = ui->left_trig;
		ui->old_r = ui->right_trig;
	
		int toty, totx;
		getmaxyx(ui->main_win, toty, totx);
		
		if (totx > 163)
		{
			ui->cx = (ui->radius + 35);
			if (toty > 50)
			{
				wheel_init(ui->main_win, ui, 7, 0, 0);
				ui->cx = (ui->radius + 40);
			}
		}
		else
		{
			ui->cx = (ui->radius + 5);
			ui->left_trig = 0;
			ui->right_trig = 0;
		
			hide_panel(ui->left_panel);
			hide_panel(ui->right_panel);
		}
		new_chart(cdata, pxx, ui, planet, zodiac);
	}

	if (x)
	{
		ui->left_trig = ui->old_l;
		ui->right_trig = ui->old_r;
		wheel_init(ui->main_win, ui, 0, 0, 0);
		new_chart(cdata, pxx, ui, planet, zodiac);
	}
}

int table_trigger(struct ui *ui, int ch)
{
	int up = 0;
	if (ch == 'p' && ui->left_trig == 0)
	{
		ui->left_trig = 1;
		show_panel(ui->left_panel);
		up = 1;
	}
	else if (ch == 'p' && ui->left_trig == 1)
	{
		ui->left_trig = 0;
		hide_panel(ui->left_panel);
		up = 1;
	}
	if (ch == 'o' && ui->right_trig == 0)
	{
		ui->right_trig = 1;
		show_panel(ui->right_panel);
		up = 1;
	}
	else if (ch == 'o' && ui->right_trig == 1)
	{
		ui->right_trig = 0;
		hide_panel(ui->right_panel);
		up = 1;
	}
	if (up)
	{
		update_panels();
		doupdate();
		return 1;
	}
	return 0;
}

static int bound_check(int sign, int degree)
{
	const int ari[] =
	{ 5, 11, 19, 24, 29 };
	const int tau[] =
	{ 7, 13, 21, 26, 29 };
	const int gem[] =
	{ 5, 11, 16, 24, 29 };
	const int can[] =
	{ 6, 12, 18, 25, 29 };
	const int leo[] =
	{ 5, 10, 17, 23, 29 };
	const int vir[] =
	{ 6, 16, 20, 27, 29 };
	const int lib[] =
	{ 5, 10, 18, 25, 29 };
	const int sco[] =
	{ 6, 10, 18, 23, 29 };
	const int sag[] =
	{ 11, 16, 20, 25, 29 };
	const int cap[] =
	{ 6, 13, 21, 25, 29 };
	const int aqu[] = 
	{ 6, 12, 19, 24, 29 };
	const int pis[] = 
	{ 11, 15, 18, 27, 29 };
	
	const int *z[] = { 0,
	ari, tau, gem, can,
	leo, vir, lib, sco,
	sag, cap, aqu, pis };
	
	for (int i = 0; i < 5; ++i)
		if (degree <= z[sign][i])
			return i + BOUND0;
	return -1;
}

static void dignity_check(int *zodiac[], double *planet[], int result[PLMAX][MAXZXX], struct pxx *pxx)
{
	int ipl = 0;
	for (; ipl < PLMAX; ++ipl)
	{
		int sign = (int)(planet[ipl][LONG] / 30) + 1;
		int degree = (int)planet[ipl][DEGREE];
		
		result[ipl][RULER] = zodiac[sign][RULER];
		result[ipl][EXALT] = zodiac[sign][EXALT];
		result[ipl][FALL] = zodiac[sign][FALL];
		result[ipl][DETRI] = zodiac[sign][DETRI];
		
		int chart_sect = sect(pxx);
		if (chart_sect == DAY_SECT)
			result[ipl][TRIPLD] = zodiac[sign][TRIPLD];
		else
			result[ipl][TRIPLD] = zodiac[sign][TRIPLN];
			
		result[ipl][BOUND0] = zodiac[sign][bound_check(sign, degree)];
		
		if (degree <= 9)
			result[ipl][DECAN0] = zodiac[sign][DECAN0];
		else if (degree > 9 && degree <= 19)
			result[ipl][DECAN0] = zodiac[sign][DECAN1];
		else if (degree > 19)
			result[ipl][DECAN0] = zodiac[sign][DECAN2];
		else // error
			result[ipl][DECAN0] = EMPTY;
	}
}

static void mutual_reception(WINDOW *win, int starty, int planet, int result[PLMAX][MAXZXX])
{
	for (int c = 0; c < PLMAX; ++c)
	{
		if (result[planet][RULER] == c && result[c][RULER] == planet && c != planet)
		{
			wattron(win, COLOR_PAIR(AIR));
			mvwprintw(win, starty, 5, "+");
			wattroff(win, COLOR_PAIR(AIR));
		}
	}
}

void left_table(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac)
{
	if (ui->cc == TRANSIT)
		return;
	const char *name[17] = { 
	"su", "mo", "me", "ve",
	"ma", "ju", "sa", "ur",
	"ne", "pl", "so", "no",
	"as", "mc", "ds", "ic",
	"  "};
	
	int p_count = 18;
	
	mvwin(ui->left_win, 0, 0);
	wresize(ui->left_win, 44, 33);
	
	werase(ui->left_win);
	
	int starty = 1, startx = 1;
	int j = 0;
	
	for (int i = 0; i < p_count; ++i)
	{
		int sign = ((int)planet[i][LONG] / 30) + 1;
		
		int full_deg = (int)planet[i][LONG];
		int deg = (int)planet[i][LONG] % 30;
		int minute = (int)((planet[i][LONG] - (int)planet[i][LONG]) * 60);
		
		if (i < 12) // sun -> north node 
		{
			char buff[MAXBUF];
			
			snprintf(buff, sizeof(buff),
			"%-3s %3d.%02d : %6s %02d*%02d`",
			name[i], full_deg, minute,
			ui->sym.pl_sym[i], deg, minute);
			
			mvwprintw(ui->left_win, starty, startx, "%s ", buff);
			
			if (planet[i][RETRO] > 0)
			{
				wattron(ui->left_win, COLOR_PAIR(FIRE));
				mvwprintw(ui->left_win, starty, startx + 12, "r");
				wattroff(ui->left_win, COLOR_PAIR(FIRE));
			}
				
			if ((int)planet[i][STATION] == STATION_R)
			{
				wattron(ui->left_win, COLOR_PAIR(EARTH));
				mvwaddstr(ui->left_win, starty, startx + 12, "sr");
				wattroff(ui->left_win, COLOR_PAIR(EARTH));
			}
			else if ((int)planet[i][STATION] == STATION_D)
			{
				wattron(ui->left_win, COLOR_PAIR(EARTH));
				mvwaddstr(ui->left_win, starty, startx + 12, "sd");
				wattroff(ui->left_win, COLOR_PAIR(EARTH));
			}
			
			int color_x = startx + (int)strlen(buff) + 1;
			zo_color(ui->left_win, ui, starty, color_x, sign, zodiac);
			
			starty += 1;
		}
		
		else if (i >= 12) // asc -> ic
		{
			if (i == 12 || i == 16)
			{
				mvwprintw(ui->left_win, starty, startx,
				"------------------------------");
				starty += 1;
			}
			
			const char *points[] = {
			"as", "mc", "ds", "ic", "fortune", "spirit"};
			
			char point_buff[MAXBUF];
			
			snprintf(point_buff, sizeof(point_buff),
			"%-10s %3d.%02d : %02d*%02d`",
			points[j], full_deg, minute, deg, minute);
			
			mvwprintw(ui->left_win, starty, startx, "%s", point_buff);
			
			int color_x = startx + (int)strlen(point_buff) + 1;
			zo_color(ui->left_win, ui, starty, color_x, sign, zodiac);
	
			starty += 1;
			j++;
		}
	}
	mvwprintw(ui->left_win, starty, startx,
	"------------------------------");
	++starty;
			
	mvwprintw(ui->left_win, starty, startx, "pl:rul:exa:tri:bou:dec:det:fal:");
	++starty;
	mvwprintw(ui->left_win, starty, startx,
	"--:---:---:---:---:---:---:-xx");
	++starty;
	
	int ipl = 0;
	while (ipl < PLMAX)
	{
		int result[PLMAX][MAXZXX] = {0};
		dignity_check(zodiac, planet, result, pxx);
	
		int dig[] = { RULER, EXALT, TRIPLD,
		BOUND0, DECAN0, DETRI, FALL };
		
		if (ipl == 12)
		{
			mvwprintw(ui->left_win, starty, startx,
			"--:---:---:---:---:---:---:---");
			++starty;
		}
		
		mvwprintw(ui->left_win, starty, startx, "%-2s:", 
		name[ipl]);
		startx += 4;
		for (int i = 0; i < 7; ++i)
		{
			if (ipl == result[ipl][dig[i]] && i < 5)
			{
				wattron(ui->left_win, COLOR_PAIR(EARTH));
				mvwprintw(ui->left_win, starty, startx, "%-2s", 
				name[result[ipl][dig[i]]]);
				wattroff(ui->left_win, COLOR_PAIR(EARTH));
				
				startx += 2;
				mvwprintw(ui->left_win, starty, startx, ":");
				startx += 2;
			}
			else if (ipl == result[ipl][dig[i]] && i >= 5)
			{
				wattron(ui->left_win, COLOR_PAIR(FIRE));
				mvwprintw(ui->left_win, starty, startx, "%-2s", 
				name[result[ipl][dig[i]]]);
				wattroff(ui->left_win, COLOR_PAIR(FIRE));
				
				startx += 2;
				mvwprintw(ui->left_win, starty, startx, ":");
				startx += 2;
			}
			else
			{
				mvwprintw(ui->left_win, starty, startx, "%-2s:", 
				name[result[ipl][dig[i]]]);
				startx += 4;
			}
			mutual_reception(ui->left_win, starty, ipl, result);
		}
		startx = 1;
		++starty;
		++ipl;
	}
	mvwprintw(ui->left_win, starty, startx,
	"------------------------------");
	
	++starty;
	
	mvwprintw(ui->left_win, starty, startx, 
	"moon phase: %s", ui->sym.moon[cdata->moonphase]);
}

void right_table(struct cdata *cdata, struct ui *ui, double *planet[], int *zodiac[])
{
	if (ui->cc == TRANSIT)
		return;
	size_t p_count = 10;
	
	mvwin(ui->right_win, LINES - 11, COLS - 24);
	wresize(ui->right_win, 11, 24);
	
	werase(ui->right_win);
	
	mvwprintw(ui->right_win, 0, 0, "(()");
	zo_color(ui->right_win, ui, 0, 4, (int)cdata->le[EN_SIGN], zodiac);
	mvwprintw(ui->right_win, 0, 8, "%2.f", cdata->le[EN_JUL]);
 
	zo_color(ui->right_win, ui, 0, 13, (int)cdata->le[EP_SIGN], zodiac);
	mvwprintw(ui->right_win, 0, 17, "-%2.f", cdata->le[EP_JUL]);
	
	mvwprintw(ui->right_win, 1, 0, "(o)");
	zo_color(ui->right_win, ui, 1, 4, (int)cdata->se[EN_SIGN], zodiac);
	mvwprintw(ui->right_win, 1, 8, "%2.f", cdata->se[EN_JUL]);
 
	zo_color(ui->right_win, ui, 1, 13, (int)cdata->se[EP_SIGN], zodiac);
	mvwprintw(ui->right_win, 1, 17, "-%2.f", cdata->se[EP_JUL]);
   
    size_t i = 2, j = SE_MERCURY;
	for (; i < p_count; ++i, ++j)
	{
		char header[MAXBUF];
		snprintf(header, sizeof(header), "%-6s%6s  %4s %4s", 
		"planet", "speed", "next", "prev");
		mvwprintw(ui->right_win, 2, 0, "%s", header);
	
		char buff[MAXBUF];
		
		if (planet[j][LONG_S] > 0.0)
		{
			snprintf(buff, sizeof(buff), "%-6s%2.0f*%-2.02d'  %-4.0f %-4.0f",
			ui->sym.pl_sym[j], planet[j][DEGREE_S], (int)planet[j][MIN_S],
			planet[j][NEXT_S], planet[j][PREV_S]);
		}
		
		else
		{
			snprintf(buff, sizeof(buff), "%-6s-%1.0f*%-2.02d'  %-4.0f %-4.0f",
			ui->sym.pl_sym[j], planet[j][DEGREE_S], (int)planet[j][MIN_S],
			planet[j][NEXT_S], planet[j][PREV_S]);
		}
		
		mvwprintw(ui->right_win, (int)i + 1, 0, "%s", buff);
	}
}
