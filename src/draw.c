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

#include <math.h>
#include <time.h>
#include <swephexp.h>
#include <ncurses.h>
#include <panel.h>
#include "astro.h"
#include "draw.h"
#include "chronos.h"

// sun, moon, mercury, venus, mars, jupiter,
// saturn, uranus, neptune, pluto, mean node, true node
const char *pl_sym[] = {"(o)", "(()", "(-o<)",
"(~:o)", "(o->)", "(\\+)", "(h)", "(\\*/)", "(?)",
"(P)", NULL, "(^)"};

// 0 = NULL because the swiss ephemeris skips 0
const char *zo_sym[] = {NULL, "ari", "tau", "gem", "can",
"leo", "vir", "lib", "sco", "sag",
"cap", "aqu", "pis"};

const char *moon[] = {"new", "crescent", "1st quarter", "gibbous", "full",
"dissem.", "4th quarter", "balsamic"};

void element_color(WINDOW *win, int y, int x, int count,
int sign, struct pxx *pxx, char ch)
{
	double *p_arr[] = {
			pxx->dsun, pxx->dmoon,
			pxx->dmerc, pxx->dven,
			pxx->dmars, pxx->djup,
			pxx->dsat, pxx->dura,
			pxx->dnep, pxx->dplu,
			pxx->dmnod, pxx->dtnod,
			pxx->dasc, pxx->dmc, 
			pxx->ddsc, pxx->dic};
			
	static int j;
	
	if (strcmp("ari", zo_sym[sign]) == 0)
		j = FIRE;
	else if (strcmp("tau", zo_sym[sign]) == 0)
		j = EARTH;
	else if (strcmp("gem", zo_sym[sign]) == 0)
		j = AIR;
	else if (strcmp("can", zo_sym[sign]) == 0)
		j = WATER;
	else if (strcmp("leo", zo_sym[sign]) == 0)
		j = FIRE;
	else if (strcmp("vir", zo_sym[sign]) == 0)
		j = EARTH;
	else if (strcmp("lib", zo_sym[sign]) == 0)
		j = AIR;
	else if (strcmp("sco", zo_sym[sign]) == 0)
		j = WATER;
	else if (strcmp("sag", zo_sym[sign]) == 0)
		j = FIRE;
	else if (strcmp("cap", zo_sym[sign]) == 0)
		j = EARTH;
	else if (strcmp("aqu", zo_sym[sign]) == 0)
		j = AIR;
	else if (strcmp("pis", zo_sym[sign]) == 0)
		j = WATER;

	switch(ch)
	{
		case 'z': // zodiac
			switch(j)
			{
				case FIRE:
					wattron(win, COLOR_PAIR(FIRE));
					mvwaddstr(win, y, x, zo_sym[sign]);
					wattroff(win, COLOR_PAIR(FIRE));
					break;
				case EARTH:
					wattron(win, COLOR_PAIR(EARTH));
					mvwaddstr(win, y, x, zo_sym[sign]);
					wattroff(win, COLOR_PAIR(EARTH));
					break;
				case AIR:
					wattron(win, COLOR_PAIR(AIR));
					mvwaddstr(win, y, x, zo_sym[sign]);
					wattroff(win, COLOR_PAIR(AIR));
					break;
				case WATER:
					wattron(win, COLOR_PAIR(WATER));
					mvwaddstr(win, y, x, zo_sym[sign]);
					wattroff(win, COLOR_PAIR(WATER));
					break;
			}
			break;
		case 'd': // decimal
			switch(j)
			{
					case FIRE:
					wattron(win, COLOR_PAIR(FIRE));
					mvwprintw(win, y, x, "%.2f", p_arr[count][MWIN]);
					wattroff(win, COLOR_PAIR(FIRE));
					break;
				case EARTH:
					wattron(win, COLOR_PAIR(EARTH));
					mvwprintw(win, y, x, "%.2f", p_arr[count][MWIN]);
					wattroff(win, COLOR_PAIR(EARTH));
					break;
				case AIR:
					wattron(win, COLOR_PAIR(AIR));
					mvwprintw(win, y, x, "%.2f", p_arr[count][MWIN]);
					wattroff(win, COLOR_PAIR(AIR));
					break;
				case WATER:
					wattron(win, COLOR_PAIR(WATER));
					mvwprintw(win, y, x, "%.2f", p_arr[count][MWIN]);
					wattroff(win, COLOR_PAIR(WATER));
					break;
			}
			break;
	}
}

void draw_circle(WINDOW *main_win, int radius, chtype ch)
{
	int center_x = COLS / 2;
	int center_y = LINES / 2;
	
	int x = 0;
	int y = radius;
	int d = 3 -2 * radius;
	
	while (x <= y)
	{
		mvwaddch(main_win, center_y + y / 2, center_x + x, ch);
		mvwaddch(main_win, center_y + y / 2, center_x - x, ch);
		mvwaddch(main_win, center_y - y / 2, center_x + x, ch);
		mvwaddch(main_win, center_y - y / 2, center_x - x, ch);
		
		mvwaddch(main_win, center_y + x / 2, center_x + y, ch);
		mvwaddch(main_win, center_y + x / 2, center_x - y, ch);
		mvwaddch(main_win, center_y - x / 2, center_x + y, ch);
		mvwaddch(main_win, center_y - x / 2, center_x - y, ch);
	
		if (d < 0)
			d = d + 4 * x + 6;
		else
		{
			d = d + 4 * (x - y) + 10;
			y--;
		}
		x++;
	}
}

void planet_pos(WINDOW *main_win,
double cusps[], int radius, struct pxx *pxx)
{
	int sign_num = (int)(cusps[1] / 30.0);
	double asc = sign_num * 30.0;
	
	double *p_arr[] = {
		pxx->dsun, pxx->dmoon,
		pxx->dmerc, pxx->dven,
		pxx->dmars, pxx->djup,
		pxx->dsat, pxx->dura,
		pxx->dnep, pxx->dplu,
		pxx->dmnod, pxx->dtnod};
		
	int center_x = (COLS / 2);
	int center_y = (LINES / 2);
	
	int iter_count = 10;
	int max_distance = 10;
	double convergence_thresh = 0.1;
	int pcount = 12;

	double adjusted_long[pcount];
	for (int i = 0; i < pcount; ++i)
		adjusted_long[i] = p_arr[i][LONG];
		
	for (int iter = 0; iter < iter_count; ++iter)
	{
		double max_change = 0.0;
		
		for (int i = 0; i < pcount; ++i)
		{
			double current = adjusted_long[i];
			double angle_offset = 0.0;
			
			for (int j = 0; j < pcount; ++j)
			{
				if (i != j)
				{
					double signed_distance = adjusted_long[j] - current;
					while (signed_distance > 180)
						signed_distance -= 360;
					while (signed_distance < -180)
						signed_distance += 360;
					double angle_distance = fabs(signed_distance);
					
					if (angle_distance < max_distance)
					{
						double strength = 
						(max_distance - angle_distance) / max_distance;
						
						int place = (signed_distance > 0) ? -1 : 1;
						
						angle_offset += 2.5 * strength * place;
					}
				}
			}
			double new_long = current + angle_offset;
			
			while (new_long < 0.0)
				new_long += 360.0;
			while (new_long >= 360.0)
				new_long -= 360.0;
		
			double change = fabs(new_long - current);
			max_change = (change > max_change) ? change : max_change;
			
			adjusted_long[i] = new_long;
		}
		if (max_change < convergence_thresh)
			break;
	}
	
	for (int i = 0; i < pcount; ++i)
	{
		double angle_rad = (adjusted_long[i] - asc) * M_PI / 180;
		double cos_rad = cos(angle_rad);
		double sin_rad = sin(angle_rad);
		
		int x = center_x - (int)(radius * cos_rad);
		int y = center_y + (int)(radius * sin_rad * 0.5);
		
		double decimal  = 
		(((p_arr[i][LONG] - (int)p_arr[i][LONG]) * 60) / 100);
		
		int sign = ((int)p_arr[i][LONG] / 30) + 1;
		
		p_arr[i][MWIN] = ((int)p_arr[i][LONG] % 30) + decimal;
		
		if (i != SE_MEAN_NODE)
		{
			element_color(main_win, y-1, x, i, sign, pxx, 'd');
			mvwaddstr(main_win, y, x, pl_sym[i]);
		
			if (p_arr[i][RETRO] > 0 && i != SE_TRUE_NODE)
			{
				wattron(main_win, COLOR_PAIR(FIRE));
				mvwprintw(main_win, y, x-1, "r");
				wattroff(main_win, COLOR_PAIR(FIRE));
			}
				
			else if (p_arr[i][LONG_S] > -0.005 &&
			p_arr[i][LONG_S] < 0.005 && i != SE_TRUE_NODE)
			{
				wattron(main_win, COLOR_PAIR(EARTH));
				mvwaddstr(main_win, y, x-1, "s");
				wattroff(main_win, COLOR_PAIR(EARTH));
			}
		}
	}
}

void ascmc_pos(WINDOW *main_win,
double cusps[], int radius, struct pxx *pxx)
{
	const char *ascmc_sym[] = {"as", "mc", "dsc", "ic"};
	
	double *asc_arr[] = { 
		pxx->dasc, pxx->dmc,
		pxx->ddsc, pxx->dic
	};
	
	for (int i = 0; i < 4; ++i)
	{
		int center_x = (COLS / 2);
		int center_y = (LINES / 2);
		
		double rad = (asc_arr[i][LONG] - cusps[1]) * M_PI / 180.0;
		
		int x = center_x - (int)(radius * cos(rad));
		int y = center_y + (int)(radius * sin(rad) * 0.5);
		
		mvwaddstr(main_win, y, x, ascmc_sym[i]);
		
		double decimal = 
		(((asc_arr[i][LONG] - (int)asc_arr[i][LONG]) * 60) / 100);
		double b60 = (((int)asc_arr[i][LONG] % 30) + decimal);
		
		asc_arr[i][MWIN] = b60;
		
		int sign = ((int)asc_arr[i][LONG] / 30) + 1;
		
		element_color(main_win, y-1, x, i + 12, sign, pxx, 'd');
	}
}

void zo_pos(WINDOW *main_win,
double cusps[], int radius, struct pxx *pxx)
{
	int asc_sign = (int)(pxx->dasc[LONG] / 30);
	for (int i = 1; i < 13; ++i)
	{
		int sign = ((i + asc_sign - 1) % 12);
		if (sign == 0)
			sign = 12;

		int center_x = (COLS / 2);
		int center_y = (LINES / 2);
		
		int sign_inc = (((int)pxx->dasc[LONG] / 30) * 30) + 15;
		
		double rad = (cusps[i] - sign_inc) * M_PI / 180.0;
		
		int x = center_x - (int)(radius * cos(rad));
		int y = center_y + (int)(radius * sin(rad) * 0.5);
		
		element_color(main_win, y, x, i, sign, pxx, 'z');
	}
}

void draw_house(WINDOW *main_win,
double cusps[], int radius, chtype ch)
{
	for (int i = 0; i < 13; ++i)
	{
		double rad = cusps[i] * M_PI / 180.0;
		
		int center_x = COLS / 2;
		int center_y = LINES / 2;
		
		int edge_x = center_x - (int)(radius * cos(rad));
		int edge_y = center_y + (int)(radius * sin(rad) * 0.5);
		
		int half_x = center_x - (int)((radius / 2) * cos(rad));
		int half_y = center_y + (int)((radius / 2)  * sin(rad) * 0.5);
		
		int dx = edge_x - half_x;
		int dy = edge_y - half_y;
		
		int distance = (int)sqrt(dx * dx + dy * dy);
		if (distance == 0)
			distance = 1;
		
		for(int j = 0; j <= distance; j++)
		{
			int x = half_x + (dx * j) / distance;
			int y = half_y + (dy * j) / distance;
			mvwaddch(main_win, y, x, ch);
		}
	}
}

void draw_chart(WINDOW *main_win, double cusps[], struct pxx *pxx)
{
	curs_set(0);
	werase(main_win);
	int radius = ((COLS / 2 < LINES) ? COLS / 2 : LINES) - 5;
	
	// zodiac
	draw_circle(main_win, radius + 4, '`');
	// out
	draw_circle(main_win, radius, '.');
	// in
	draw_circle(main_win, (radius / 2) - 1, '.');
	
	draw_house(main_win, cusps, radius + 4, '`');
	
	zo_pos(main_win, cusps, radius + 3, pxx);
	
	planet_pos(main_win, cusps, radius - 5, pxx);
	
	ascmc_pos(main_win, cusps, (radius / 2) + 4, pxx);
}

void cur_chart_data(WINDOW *main_win, struct io *io,
struct cdata *cdata)
{	
	int starty = 3;
	int startx = COLS - 22;
	
	if(io->filename)
		mvwprintw(main_win, starty, startx, "%s", io->filename);
	
	starty += 1;
	if(cdata->city)
		mvwprintw(main_win, starty, startx, "%s", cdata->city);
	
	starty += 1;
	if(cdata->tm_year)
		mvwprintw(main_win, starty, startx, "%d", cdata->tm_year);
	
	starty += 1;
	if(cdata->tm_mon && cdata->tm_mday)
		mvwprintw(main_win, starty, startx, "%02d/%02d",
		cdata->tm_mon, cdata->tm_mday);
		
	starty += 1;
	if (cdata->tm_hour >= 0)
		mvwprintw(main_win, starty, startx, "%02d", cdata->tm_hour);
	
	startx += 2;
	if(cdata->tm_min >= 0)
		mvwprintw(main_win, starty, startx, ":%02d", cdata->tm_min);
	
	startx += 3;
	if (cdata->tm_sec >= 0)
		mvwprintw(main_win, starty, startx, ":%02d", cdata->tm_sec);
	startx -= 5;
	
	starty += 1;
	if (fabs(cdata->dlat) > 1e-6)
		mvwprintw(main_win, starty, startx, "lat.%f", cdata->dlat);
	
	starty += 1;
	if (fabs(cdata->dlon) > 1e-6)
		mvwprintw(main_win, starty, startx, "lon.%f", cdata->dlon);
}

int moon_phase(struct pxx *pxx)
{
	double elongation = pxx->dmoon[LONG] - pxx->dsun[LONG];
	
	while (elongation < 0)
		elongation += 360;
	while (elongation >= 360)
		elongation -= 360;
		
	int phase = (int)(elongation / 45);
	if (phase > 7)
		phase = 7;
		
	return phase;
}

void planet_table(WINDOW *planet_win, struct pxx *pxx)
{
	char spname[AS_MAXCH];
	int p_count = 18;
	
	double *p_arr[] = {
		pxx->dsun, pxx->dmoon,
		pxx->dmerc, pxx->dven,
		pxx->dmars, pxx->djup,
		pxx->dsat, pxx->dura,
		pxx->dnep, pxx->dplu,
		pxx->dmnod, pxx->dtnod,
		pxx->dfor, pxx->dspir,
		pxx->dasc, pxx->dmc,
		pxx->ddsc, pxx->dic};
		
	werase(planet_win);
	
	int starty = 1, startx = 2;
	int j = 0;
	
	for (int i = 0; i < p_count; ++i)
	{
		int sign = ((int)p_arr[i][LONG] / 30) + 1;
		
		int full_deg = (int)p_arr[i][LONG];
		int deg = (int)p_arr[i][LONG] % 30;
		int a_dec = (int)((p_arr[i][LONG] - (int)p_arr[i][LONG]) * 60);
		
		if ( i != SE_MEAN_NODE && i < 12) // sun -> node 
		{
			swe_get_planet_name(i, spname);
			spname[2] ='\0';
			
			char buff[MAXBUF];
			
			
			snprintf(buff, sizeof(buff),
			"%-3s %3d.%02d : %6s %02d*%02d`",
			spname, full_deg, a_dec,
			pl_sym[i], deg, a_dec);
			
			mvwprintw(planet_win, starty, startx, "%s ", buff);
			
			if (p_arr[i][RETRO] > 0 && i != SE_TRUE_NODE)
			{
				wattron(planet_win, COLOR_PAIR(FIRE));
				mvwprintw(planet_win, starty, startx + 13, "r");
				wattroff(planet_win, COLOR_PAIR(FIRE));
			}
				
			else if (p_arr[i][LONG_S] > -0.01 &&
			p_arr[i][LONG_S] < 0.01 && i != SE_TRUE_NODE)
			{
				wattron(planet_win, COLOR_PAIR(EARTH));
				mvwaddstr(planet_win, starty, startx + 13, "s");
				wattroff(planet_win, COLOR_PAIR(EARTH));
			}
			
			int color_x = startx + (int)strlen(buff) + 1;
			element_color(planet_win, starty, color_x, 1, sign, pxx, 'z');
			
			starty += 2;
		}
		
		else if ( i != SE_MEAN_NODE && i >= 12) // asc -> ic
		{
			const char *points[] = {
			"fortune", "spirit", "asc", "mc", "dsc", "ic"};
			
			char point_buff[MAXBUF];
			
			snprintf(point_buff, sizeof(point_buff),
			"%-10s %3d.%02d : %02d*%02d`",
			points[j], full_deg, a_dec, deg, a_dec);
			
			if (i == 12) // lots divider
			{
				mvwprintw(planet_win, starty, startx,
				"------------------------------");
				starty += 2;
			}
			
			if (i == 14) // points divider
			{
				mvwprintw(planet_win, starty, startx,
				"------------------------------");
				starty += 2;
			}
			mvwprintw(planet_win, starty, startx, "%s", point_buff);
			
			int color_x = startx + (int)strlen(point_buff) + 1;
			element_color(planet_win, starty, color_x, 1, sign, pxx, 'z');
	
			starty += 2;
			mvwprintw(planet_win, starty, startx, 
			"moon phase: %s", moon[moon_phase(pxx)]);
			++j;
		}
	}
}

void retro_table(WINDOW *retro_win, struct pxx *pxx)
{
	double *p_arr[] = {
		pxx->dmerc, pxx->dven,
		pxx->dmars, pxx->djup,
		pxx->dsat, pxx->dura,
		pxx->dnep, pxx->dplu};
		
	size_t p_count = 8;
	
	werase(retro_win);
	
        
	for (size_t i = 0; i < p_count; ++i)
	{
		char header[MAXBUF];
		snprintf(header, sizeof(header), "%-6s %7s %4s %4s", 
		"x---x-", "speed", "next", "last");
		mvwprintw(retro_win, 0, 0, "%s", header);
	
		char buff[MAXBUF];
		
		snprintf(buff, sizeof(buff), "%-6s %7.3f %4.0f %4.0f",
		pl_sym[i+2], p_arr[i][LONG_S], p_arr[i][NEXT_R], p_arr[i][LAST_R]);
		
		mvwprintw(retro_win, (int)i + 1, 0, "%s", buff);
	}
}

void new_chart(WINDOW *main_win, WINDOW *planet_win, WINDOW *retro_win,
PANEL **planet_panel, PANEL **retro_panel,
struct io *io, struct cdata *cdata, struct pxx *pxx,
int *planet_trig, int *retro_trig, double cusps[])
{
	pxx_fill(cusps, cdata, pxx);
	draw_chart(main_win, cusps, pxx);
	cur_chart_data(main_win, io, cdata);
	
	if (*planet_trig > 0)
	{
		planet_table(planet_win, pxx);
		show_panel(*planet_panel);
	}
	if (*retro_trig > 0)
	{
		retro_table(retro_win, pxx);
		show_panel(*retro_panel);
	}	
	update_panels();
}

void realtime_chart(WINDOW *main_win, WINDOW *planet_win, WINDOW *retro_win,
PANEL **planet_panel, PANEL **retro_panel,
struct io *io, struct cdata *cdata, struct pxx *pxx,
int *planet_trig, int *retro_trig, double cusps[])
{
	nodelay(main_win, TRUE);
	struct tm gettime = {0};
		
	int ch = 0;
	while ((ch = wgetch(main_win)) != 9)
	{
		time_t now = time(NULL);
		localtime_r(&now, &gettime);
		
		cdata->tm_year = gettime.tm_year+1900;
		cdata->tm_mon = gettime.tm_mon + 1;
		cdata->tm_mday = gettime.tm_mday;
		cdata->tm_hour = gettime.tm_hour;
		cdata->tm_min = gettime.tm_min;
		cdata->tm_sec = gettime.tm_sec;
		
		new_chart(main_win, planet_win, retro_win,
		planet_panel, retro_panel,
		io, cdata, pxx,
		planet_trig, retro_trig, cusps);
		
		wattron(main_win, COLOR_PAIR(FIRE));
		mvwprintw(main_win, 2, COLS - 22, "*live");
		wattroff(main_win, COLOR_PAIR(FIRE));
		
		wnoutrefresh(main_win);
		update_panels();
		doupdate();
	
		for (int i = 0; i < 10; ++i)
		{
			usleep(100000);
			if ((ch = wgetch(main_win)) == 9)
				break;
		}
		
		if (ch == 9)
			break;
	}
	wmove(main_win, 2, COLS - 22);
	wclrtoeol(main_win);
	nodelay(main_win, FALSE);
}
	
void animate_chart(WINDOW *main_win, WINDOW *planet_win, WINDOW *retro_win,
PANEL **planet_panel, PANEL **retro_panel,
struct io *io, struct cdata *cdata, struct pxx *pxx,
int *planet_trig, int *retro_trig, double cusps[])
{
	int starty = 11;
	int startx = COLS - 22;
	
	mvwprintw(main_win, starty, startx, "(min)");
	
	int max_day = 0; // months() return flag
	size_t i = MINUTE; // time inc/dec
	
	struct tm temp = {0};
	struct tm *result = NULL;
	
	temp.tm_year = cdata->tm_year - 1900;
	temp.tm_mon = cdata->tm_mon - 1;
	temp.tm_mday = cdata->tm_mday;
	temp.tm_hour = cdata->tm_hour;
	temp.tm_min = cdata->tm_min;
	temp.tm_sec = cdata->tm_sec;
	temp.tm_isdst = -1;
	
	time_t t = mktime(&temp);
	
	int ch = 0;
	int anim_done = 0;
	while(!anim_done && (ch = wgetch(main_win)))
	{
		switch(ch)
		{
			case 'h': case KEY_LEFT:
				if (i != MINUTE)
					++i;
				break;
				
			case 'l': case KEY_RIGHT:
				if (i != YEAR) // time inc/dec
					--i;
				break;
				
			case 'p':
				if (*planet_trig)
				{
					hide_panel(*planet_panel);
					*planet_trig = 0;
				}
				else
				{
					planet_table(planet_win, pxx);
					show_panel(*planet_panel);
					*planet_trig = 1;
				}
				
				if (*retro_trig > 0)
				{
					retro_table(retro_win, pxx);
					show_panel(*retro_panel);
				}
				
				touchwin(main_win);
				wnoutrefresh(main_win);
				update_panels();
				doupdate();
				break;
				
			case 'o':
				if (*retro_trig)
				{
					hide_panel(*retro_panel);
					*retro_trig = 0;
				}
				else
				{
					retro_table(retro_win, pxx);
					show_panel(*retro_panel);
					*retro_trig = 1;
				}
				
				if (*planet_trig > 0)
				{
					planet_table(planet_win, pxx);
					show_panel(*planet_panel);
				}
				touchwin(main_win);
				wnoutrefresh(main_win);
				update_panels();
				doupdate();
				break;
			
			case 'k': case KEY_UP:
				switch(i)
				{
					case MINUTE:
						t += 60;
						break;
					case HOUR:
						t += 3600;
						break;
					case DAY:
						t += 86400;
						break;
					case MONTH:
						if ((++temp.tm_mon) > 11)
						{
							temp.tm_mon = 0;
							++temp.tm_year;
						}
						max_day = months(temp.tm_mon, temp.tm_year);
						if (temp.tm_mday > max_day)
							temp.tm_mday = max_day;
						t = mktime(&temp);
						break;
					case YEAR:
						temp.tm_year++;
						if (temp.tm_year > 16799)
							temp.tm_year = -12998;
						t = mktime(&temp);
					break;
				}
				result = localtime(&t);
				
				cdata->tm_year = result->tm_year + 1900;
				cdata->tm_mon = result->tm_mon + 1;
				cdata->tm_mday = result->tm_mday;
				cdata->tm_hour = result->tm_hour;
				cdata->tm_min = result->tm_min;
				cdata->tm_sec = result->tm_sec;
				
				new_chart(main_win, planet_win, retro_win,
				planet_panel, retro_panel,
				io, cdata, pxx,
				planet_trig, retro_trig, cusps);
				break;
			case 'j': case KEY_DOWN:
				switch(i)
				{
					case MINUTE:
						t -= 60;
						break;
					case HOUR:
						t -= 3600;
						break;
					case DAY:
						t -= 86400;
						break;
					case MONTH:
						if ((--temp.tm_mon) < 0)
						{
							temp.tm_mon = 11;
							--temp.tm_year;
						}
						max_day = months(temp.tm_mon, temp.tm_year);
						if (temp.tm_mday > max_day)
							temp.tm_mday = max_day;
						t = mktime(&temp);
						break;
					case YEAR:
						--temp.tm_year;
						if (temp.tm_year < -12998)
							temp.tm_year = 16799;
						t = mktime(&temp);
					break;
				}
				result = localtime(&t);
				
				cdata->tm_year = result->tm_year + 1900;
				cdata->tm_mon = result->tm_mon + 1;
				cdata->tm_mday = result->tm_mday;
				cdata->tm_hour = result->tm_hour;
				cdata->tm_min = result->tm_min;
				cdata->tm_sec = result->tm_sec;
				
				new_chart(main_win, planet_win, retro_win,
				planet_panel, retro_panel,
				io, cdata, pxx,
				planet_trig, retro_trig, cusps);
	
				break;
			case '\n':
				anim_done = 1;
				break;
		}
		flushinp();
		usleep(8666);
		switch(i)
		{
			case MINUTE:
				wmove(main_win, starty, startx);
				wclrtoeol(main_win);
				mvwprintw(main_win, starty, startx, "(min)");
				break;
				
			case HOUR:
				wmove(main_win, starty, startx);
				wclrtoeol(main_win);
				mvwprintw(main_win, starty, startx, "(hour)");
				break;
				
			case DAY:
				wmove(main_win, starty, startx);
				wclrtoeol(main_win);
				mvwprintw(main_win, starty, startx, "(day)");
				break;
				
			case MONTH:
				wmove(main_win, starty, startx);
				wclrtoeol(main_win);
				mvwprintw(main_win, starty, startx, "(mon)");
				break;
				
			case YEAR:
				wmove(main_win, starty, startx);
				wclrtoeol(main_win);
				mvwprintw(main_win, starty, startx, "(year)");
				break;
		}
	}
	wmove(main_win, starty, startx);
	wclrtoeol(main_win);
	wnoutrefresh(main_win);
}
 
