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
#include "cdata.h"
#include "draw.h"
#include "chronos.h"

void zodiac_color(WINDOW *win, int y, int x, int sign,
const char *zo_sym[], int *z_arr[])
{
	switch(z_arr[sign][ELEMENT])
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
}

void degree_color(WINDOW *win, int y, int x, int count,
double *p_arr[], int *z_arr[])
{
	int sign = (int)(p_arr[count][LONG] / 30) + 1;
	
	switch(z_arr[sign][ELEMENT])
	{
		case FIRE:
			wattron(win, COLOR_PAIR(FIRE));
			mvwprintw(win, y, x, "%.0f*%02.0f`",
			p_arr[count][DEGREE], p_arr[count][MIN]);
			wattroff(win, COLOR_PAIR(FIRE));
			break;
		case EARTH:
			wattron(win, COLOR_PAIR(EARTH));
			mvwprintw(win, y, x, "%.0f*%02.0f`",
			p_arr[count][DEGREE], p_arr[count][MIN]);
			wattroff(win, COLOR_PAIR(EARTH));
			break;
		case AIR:
			wattron(win, COLOR_PAIR(AIR));
			mvwprintw(win, y, x, "%.0f*%02.0f`",
			p_arr[count][DEGREE], p_arr[count][MIN]);
			wattroff(win, COLOR_PAIR(AIR));
			break;
		case WATER:
			wattron(win, COLOR_PAIR(WATER));
			mvwprintw(win, y, x, "%.0f*%02.0f`",
			p_arr[count][DEGREE], p_arr[count][MIN]);
			wattroff(win, COLOR_PAIR(WATER));
			break;
	}
}

void planet_pos(WINDOW *main_win, double cusps[], double *p_arr[], int *z_arr[],
int radius, int center_y, int center_x, 
const char *pl_sym[])
{
	int sign_num = (int)(cusps[1] / 30.0);
	double asc = sign_num * 30.0;
	
	int iter_count = 50;
	int max_distance = 11;
	double convergence_thresh = 0.1;
	int pcount = 12;
	double str_base = 5.0;

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
						
						angle_offset += str_base * strength * place;
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
		
		p_arr[i][DEGREE] = (int)p_arr[i][LONG] % 30;
		p_arr[i][MIN] = (int)((p_arr[i][LONG] - (int)p_arr[i][LONG]) * 60);
	
		degree_color(main_win, y-1, x, i, p_arr, z_arr);
		mvwaddstr(main_win, y, x, pl_sym[i]);
	
		if (p_arr[i][RETRO] > 0 && i != SE_TRUE_NODE)
		{
			wattron(main_win, COLOR_PAIR(FIRE));
			mvwprintw(main_win, y, x-1, "r");
			wattroff(main_win, COLOR_PAIR(FIRE));
		}
			
		if ((int)p_arr[i][STATION] == STATION_R)
		{
			wattron(main_win, COLOR_PAIR(EARTH));
			mvwaddstr(main_win, y, x-2, "sr");
			wattroff(main_win, COLOR_PAIR(EARTH));
		}
		else if ((int)p_arr[i][STATION] == STATION_D)
		{
			wattron(main_win, COLOR_PAIR(EARTH));
			mvwaddstr(main_win, y, x -2, "sd");
			wattroff(main_win, COLOR_PAIR(EARTH));
		}
	}
}

void ascmc_pos(WINDOW *main_win, double cusps[], double *p_arr[], int *z_arr[],
int radius, int center_y, int center_x)
{
	const char *ascmc_sym[] = {"as", "mc", "ds", "ic"};
	
	int j = 0;
	int i = 12;
	for (j = 0; i < 16; ++i, ++j)
	{
		double rad = (p_arr[i][LONG] - cusps[1]) * M_PI / 180.0;
		
		int x = center_x - (int)(radius * cos(rad));
		int y = center_y + (int)(radius * sin(rad) * 0.5);
		
		mvwaddstr(main_win, y, x, ascmc_sym[j]);
		
		p_arr[i][DEGREE] = (int)p_arr[i][LONG] % 30;
		p_arr[i][MIN]  = (int)((p_arr[i][LONG] - (int)p_arr[i][LONG]) * 60);
		
		degree_color(main_win, y-1, x, i, p_arr, z_arr);
	}
}

void zo_pos(WINDOW *main_win, double cusps[],
int radius, int center_y, int center_x,
struct pxx *pxx, const char *zo_sym[], int *z_arr[])
{
	int asc_sign = (int)(pxx->dasc[LONG] / 30) - 1;
	for (int i = ARI; i < ZMAX; ++i)
	{
		int sign = ((i + asc_sign) % 12);
		if (sign == 0)
			sign = 12;

		int sign_inc = (((int)pxx->dasc[LONG] / 30) * 30) + 15;
		
		double rad = (cusps[i] - sign_inc) * M_PI / 180.0;
		
		int x = center_x - (int)(radius * cos(rad));
		int y = center_y + (int)(radius * sin(rad) * 0.5);
		
		zodiac_color(main_win, y, x, sign, zo_sym, z_arr);
	}
}

void draw_house(WINDOW *main_win, double cusps[],
int radius, int center_y, int center_x,
chtype ch)
{
	for (int i = ARI; i < ZMAX; ++i)
	{
		double rad = cusps[i] * M_PI / 180.0;
		
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

void draw_circle(WINDOW *main_win,
int radius, int center_y, int center_x,
chtype ch)
{
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

void draw_chart(WINDOW *main_win, double cusps[], double *p_arr[], int *z_arr[],
struct pxx *pxx, struct cdata *cdata,  const char *pl_sym[], const char *zo_sym[])
{
	curs_set(0);
	werase(main_win);
	int radius = ((COLS / 2 < LINES) ? COLS / 2 : LINES) - 5;
	
	int offsetx = 0;
	if ((COLS - LINES) > 60)
		offsetx += 7;
	else
		offsetx = 0;
		
	int center_y = (LINES / 2);
	int center_x = (COLS / 2) + offsetx;
	
	// zodiac
	draw_circle(main_win, radius + 4, center_y, center_x, '`');
	// out
	draw_circle(main_win, radius, center_y, center_x,'.');
	// in
	draw_circle(main_win, (radius / 2) - 1, center_y, center_x, '.');
	
	draw_house(main_win, cusps, radius + 4, center_y, center_x, '`');
	
	zo_pos(main_win, cusps, radius + 3, center_y, center_x, pxx,
	zo_sym, z_arr);
	
	planet_pos(main_win, cusps, p_arr, z_arr, radius - 5, center_y, center_x,
	pl_sym);
	
	if (fabs(cdata->dlat) > 1e-6)
		ascmc_pos(main_win, cusps, p_arr, z_arr, (radius / 2) + 4, center_y, center_x);
	
	// status bar
	mvwhline(main_win, 1, COLS - 15, '.', COLS);
	mvwvline(main_win, 0, COLS - 15, '.', 2);
}

void cur_chart_data(WINDOW *main_win, struct io *io,
struct cdata *cdata)
{	
	int starty = (LINES / 2) - 4;
	int startx = (COLS / 2) - 4;
	
	if(io->filename)
		mvwprintw(main_win, starty, startx, "%s", io->filename);
	
	starty += 1;
	if(cdata->city)
		mvwprintw(main_win, starty, startx, "%s", cdata->city);
		
	if(cdata->state)
	{
		if (!isdigit((unsigned char)cdata->state[0]))
		{ // when not in the usa
			startx += (int)strlen(cdata->city);
			mvwprintw(main_win, starty, startx, ", %s", cdata->state);
			startx -= (int)strlen(cdata->city);
		}
	}
	
	starty += 1;
	if(cdata->country)
		mvwprintw(main_win, starty, startx, "%s", cdata->country);
	
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

int bound_check(int sign, int degree)
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
	
	const int *zodia[] = { 0,
	ari, tau, gem, can,
	leo, vir, lib, sco,
	sag, cap, aqu, pis };
	
	for (int i = 0; i < 5; ++i)
		if (degree <= zodia[sign][i])
			return i + BOUND0;
	return -1;
}

void dignity_check(int *z_arr[], double *p_arr[], int planet, int result[],
struct pxx *pxx)
{
	int sign = (int)(p_arr[planet][LONG] / 30) + 1;
	int degree = (int)p_arr[planet][DEGREE];
	
	result[RULER] = z_arr[sign][RULER];
	result[EXALT] = z_arr[sign][EXALT];
	
	int chart_sect = sect(pxx);
	if (chart_sect == DAY_SECT)
		result[TRIPLD] = z_arr[sign][TRIPLD];
	else
		result[TRIPLD] = z_arr[sign][TRIPLN];
		
	result[BOUND0] = z_arr[sign][bound_check(sign, degree)];
	
	if (degree <= 9)
		result[DECAN0] = z_arr[sign][DECAN0];
	else if (degree > 9 && degree <= 19)
		result[DECAN0] = z_arr[sign][DECAN1];
	else if (degree > 19)
		result[DECAN0] = z_arr[sign][DECAN2];
	else // error
		result[DECAN0] = 12;
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

void planet_table(WINDOW *planet_win, double *p_arr[], int *z_arr[], struct pxx *pxx, 
const char *pl_sym[], const char *zo_sym[], const char *moon[])
{
	char *name[13] = { 
	"su", "mo", "me", "ve",
	"ma", "ju", "sa", "ur",
	"ne", "pl", "so", "no",
	"  "};
	
	int p_count = 18;
	
	mvwin(planet_win, 0, 0);
	wresize(planet_win, 40, 33);
	
	werase(planet_win);
	
	int starty = 1, startx = 2;
	int j = 0;
	
	for (int i = 0; i < p_count; ++i)
	{
		int sign = ((int)p_arr[i][LONG] / 30) + 1;
		
		int full_deg = (int)p_arr[i][LONG];
		int deg = (int)p_arr[i][LONG] % 30;
		int minute = (int)((p_arr[i][LONG] - (int)p_arr[i][LONG]) * 60);
		
		if (i < 12) // sun -> north node 
		{
			char buff[MAXBUF];
			
			snprintf(buff, sizeof(buff),
			"%-3s %3d.%02d : %6s %02d*%02d`",
			name[i], full_deg, minute,
			pl_sym[i], deg, minute);
			
			mvwprintw(planet_win, starty, startx, "%s ", buff);
			
			if (p_arr[i][RETRO] > 0)
			{
				wattron(planet_win, COLOR_PAIR(FIRE));
				mvwprintw(planet_win, starty, startx + 12, "r");
				wattroff(planet_win, COLOR_PAIR(FIRE));
			}
				
			if ((int)p_arr[i][STATION] == STATION_R)
			{
				wattron(planet_win, COLOR_PAIR(EARTH));
				mvwaddstr(planet_win, starty, startx + 12, "sr");
				wattroff(planet_win, COLOR_PAIR(EARTH));
			}
			else if ((int)p_arr[i][STATION] == STATION_D)
			{
				wattron(planet_win, COLOR_PAIR(EARTH));
				mvwaddstr(planet_win, starty, startx + 12, "sd");
				wattroff(planet_win, COLOR_PAIR(EARTH));
			}
			
			int color_x = startx + (int)strlen(buff) + 1;
			zodiac_color(planet_win, starty, color_x, sign, zo_sym, z_arr);
			
			starty += 1;
		}
		
		else if (i >= 12) // asc -> ic
		{
			if (i == 12 || i == 16)
			{
				mvwprintw(planet_win, starty, startx,
				"------------------------------");
				starty += 1;
			}
			
			const char *points[] = {
			"as", "mc", "ds", "ic", "fortune", "spirit"};
			
			char point_buff[MAXBUF];
			
			snprintf(point_buff, sizeof(point_buff),
			"%-10s %3d.%02d : %02d*%02d`",
			points[j], full_deg, minute, deg, minute);
			
			mvwprintw(planet_win, starty, startx, "%s", point_buff);
			
			int color_x = startx + (int)strlen(point_buff) + 1;
			zodiac_color(planet_win, starty, color_x, sign, zo_sym, z_arr);
	
			starty += 1;
			j++;
		}
	}
	int planet = 0;
	
	mvwprintw(planet_win, starty, startx,
	"------------------------------");
	++starty;
			
	mvwprintw(planet_win, starty, startx, "p  : ru : ex : tr : bo : de ");
	++starty;
	mvwprintw(planet_win, starty, startx,
	"...........................xx");
	++starty;
	do
	{
		int result[MAXZXX] = {0};
		dignity_check(z_arr, p_arr, planet, result, pxx);
	
		int dig[] = { RULER, EXALT, TRIPLD,
		BOUND0, DECAN0 };
		
		mvwprintw(planet_win, starty, startx, "%-2s :", 
		name[planet]);
		startx += 5;
		for (int i = 0; i < 5; ++i)
		{
			if (strcmp(name[planet], name[result[dig[i]]]) == 0)
			{
				wattron(planet_win, COLOR_PAIR(EARTH));
				mvwprintw(planet_win, starty, startx, "%-2s", 
				name[result[dig[i]]]);
				wattroff(planet_win, COLOR_PAIR(EARTH));
				
				startx += 3;
				mvwprintw(planet_win, starty, startx, ":");
				startx += 2;
			}
			else
			{
				mvwprintw(planet_win, starty, startx, "%-2s :", 
				name[result[dig[i]]]);
				startx += 5;
			}
		}
		startx = 2;
		++starty;
	}
	while (planet++ < 11);
			
	++starty;
	mvwprintw(planet_win, starty, startx, 
	"moon phase: %s", moon[moon_phase(pxx)]);
}

void retro_table(WINDOW *retro_win, double *p_arr[],
const char *pl_sym[])
{
	size_t p_count = 8;
	
	mvwin(retro_win, LINES - 9, COLS - 24);
	wresize(retro_win, 9, 24);
	
	werase(retro_win);
        
    size_t i = 0, j = SE_MERCURY;
	for (; i < p_count; ++i, ++j)
	{
		char header[MAXBUF];
		snprintf(header, sizeof(header), "%-6s%6s  %4s %4s", 
		"x---x-", "speed", "n->r", "n->s");
		mvwprintw(retro_win, 0, 0, "%s", header);
	
		char buff[MAXBUF];
		
		snprintf(buff, sizeof(buff), "%-6s%6.3f  %-4.0f %-4.0f",
		pl_sym[j], p_arr[j][LONG_S], 
		p_arr[j][NEXT_R], fabs(p_arr[j][NEXT_S]));
		
		mvwprintw(retro_win, (int)i + 1, 0, "%s", buff);
	}
}

void new_chart(NEW_CHART_PARAM())
{
	pxx_fill(cusps, p_arr, cdata, pxx);
	draw_chart(main_win, cusps, p_arr, z_arr, pxx, cdata,
	pl_sym, zo_sym);
	cur_chart_data(main_win, io, cdata);
	
	if (*planet_trig > 0)
	{
		planet_table(planet_win, p_arr, z_arr, pxx,
		pl_sym, zo_sym, moon);
		show_panel(*planet_panel);
	}
	if (*retro_trig > 0)
	{
		retro_table(retro_win, p_arr, pl_sym);
		show_panel(*retro_panel);
	}	
	update_panels();
}

void realtime_chart(NEW_CHART_PARAM())
{
	nodelay(main_win, TRUE);
		
	int ch = 0;
	while ((ch = wgetch(main_win)) != 9)
	{
		set_localtime(cdata);
		new_chart(NEW_CHART_ARG());
		
		wattron(main_win, COLOR_PAIR(FIRE));
		mvwprintw(main_win, 0, COLS - 14, "*live");
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
	wmove(main_win, 0, COLS - 14);
	wclrtoeol(main_win);
	nodelay(main_win, FALSE);
}

void cpt(struct cdata *cdata,
struct tm *temp, struct tm *result, time_t *t,
bool x)
{
	if (!x)
	{
		temp->tm_year = cdata->tm_year - 1900;
		temp->tm_mon = cdata->tm_mon - 1;
		temp->tm_mday = cdata->tm_mday;
		temp->tm_hour = cdata->tm_hour;
		temp->tm_min = cdata->tm_min;
		temp->tm_sec = cdata->tm_sec;
		temp->tm_isdst = -1;
		
		*t = mktime(temp);
	}
	if (x)
	{	
		result = localtime(t);
		cdata->tm_year = result->tm_year + 1900;
		cdata->tm_mon = result->tm_mon + 1;
		cdata->tm_mday = result->tm_mday;
		cdata->tm_hour = result->tm_hour;
		cdata->tm_min = result->tm_min;
		cdata->tm_sec = result->tm_sec;
	}
}

void solar_return(NEW_CHART_PARAM())
{
	struct tm gettime = {0};
	double base_degree = pxx->dsun[LONG];
	time_t now = time(NULL);
	localtime_r(&now, &gettime);

	int current_year = gettime.tm_year+1900;
	int diff = current_year - cdata->tm_year;
	
	wattron(main_win, COLOR_PAIR(AIR));
	mvwprintw(main_win, 0, COLS - 14, "*solar return");
	wattroff(main_win, COLOR_PAIR(AIR));
	
	int solar_done = 0, ch = 'f', first_run = 1;
	while (!solar_done)
	{
		if (!first_run)
		{
			ch = wgetch(main_win);
			if (!ch)
				break;
		}
		else 
			first_run = 0;
			
		switch(ch)
		{
			case 'f':
				cdata->tm_year += diff;
				ch = 0;
				break;
			case 'j': case KEY_DOWN:
				--cdata->tm_year;
				break;
			case 'k': case KEY_UP:
				++cdata->tm_year;
				break;
			case 'q': case 's': case 27:
				solar_done = 1;
				break;
		}
		flushinp();
		
		pxx_fill(cusps, p_arr, cdata, pxx);
		
		double temp_degree = pxx->dsun[LONG];
		int iter = 3;
		
		while (--iter > 0)
		{
			while (temp_degree < base_degree)
			{
				if (base_degree - temp_degree > 1.0)
					++cdata->tm_mday;
					
				else if (base_degree - temp_degree > 0.02)
					++cdata->tm_hour;
					
				else if (base_degree - temp_degree > 0.0006)
					++cdata->tm_min;
				else
					++cdata->tm_sec;
					
				new_chart(NEW_CHART_ARG());
				temp_degree = pxx->dsun[LONG];
		
				wattron(main_win, COLOR_PAIR(AIR));
				mvwprintw(main_win, 0, COLS - 14, "*solar return");
				wattroff(main_win, COLOR_PAIR(AIR));
			}
			while (temp_degree > base_degree)
			{
				if (temp_degree - base_degree > 1.0)
					--cdata->tm_mday;
					
				else if (temp_degree - base_degree > 0.02)
					--cdata->tm_hour;
				else if (temp_degree - base_degree > 0.0006)
					--cdata->tm_min;
				else
					--cdata->tm_sec;
					
				new_chart(NEW_CHART_ARG());
				temp_degree = pxx->dsun[LONG];
				
				wattron(main_win, COLOR_PAIR(AIR));
				mvwprintw(main_win, 0, COLS - 14, "*solar return");
				wattroff(main_win, COLOR_PAIR(AIR));
			}
			struct tm temp = {0};
			struct tm *result = NULL;
			time_t t = 0;
	
			cpt(cdata, &temp, result, &t, 0);
			cpt(cdata, &temp, result, &t, 1);
			
			pxx_fill(cusps, p_arr, cdata, pxx);
			cur_chart_data(main_win, io, cdata);
		}
	}
	mvwprintw(main_win, 0, COLS - 14, "              ");
}

void animate_chart(NEW_CHART_PARAM())
{
	int starty = 0;
	int startx = COLS - 14;
	
	mvwprintw(main_win, starty, startx, "(min)");
	
	int max_day = 0; // months() return flag
	size_t i = MINUTE; // time inc/dec
	
	struct tm temp = {0};
	struct tm *result = NULL;
	time_t t = 0;
	
	cpt(cdata, &temp, result, &t, 0);
	
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
					planet_table(planet_win, p_arr, z_arr, pxx,
					pl_sym, zo_sym, moon);
					show_panel(*planet_panel);
					*planet_trig = 1;
				}
				
				if (*retro_trig > 0)
				{
					retro_table(retro_win, p_arr, pl_sym);
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
					retro_table(retro_win, p_arr, pl_sym);
					show_panel(*retro_panel);
					*retro_trig = 1;
				}
				
				if (*planet_trig > 0)
				{
					planet_table(planet_win, p_arr, z_arr, pxx,
					pl_sym, zo_sym, moon);
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
				cpt(cdata, &temp, result, &t, 1);
				
				new_chart(NEW_CHART_ARG());
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
				cpt(cdata, &temp, result, &t, 1);
				
				new_chart(NEW_CHART_ARG());
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
