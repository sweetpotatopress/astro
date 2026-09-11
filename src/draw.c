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

#include <math.h>
#include <ncurses.h>
#include <panel.h>
#include "swephexp.h"
#include "astro.h"
#include "draw.h"
#include "chronos.h"
#include "table.h"

void zo_color(WINDOW *win, int y, int x, int sign,
const char *zo_sym[], int *zodiac[])
{
	switch(zodiac[sign][ELEMENT])
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
double *planet[], int *zodiac[])
{
	int sign = (int)(planet[count][LONG] / 30) + 1;
	
	switch(zodiac[sign][ELEMENT])
	{
		case FIRE:
			wattron(win, COLOR_PAIR(FIRE));
			mvwprintw(win, y, x, "%.0f*%02.0f",
			planet[count][DEGREE], planet[count][MIN]);
			wattroff(win, COLOR_PAIR(FIRE));
			break;
		case EARTH:
			wattron(win, COLOR_PAIR(EARTH));
			mvwprintw(win, y, x, "%.0f*%02.0f",
			planet[count][DEGREE], planet[count][MIN]);
			wattroff(win, COLOR_PAIR(EARTH));
			break;
		case AIR:
			wattron(win, COLOR_PAIR(AIR));
			mvwprintw(win, y, x, "%.0f*%02.0f",
			planet[count][DEGREE], planet[count][MIN]);
			wattroff(win, COLOR_PAIR(AIR));
			break;
		case WATER:
			wattron(win, COLOR_PAIR(WATER));
			mvwprintw(win, y, x, "%.0f*%02.0f",
			planet[count][DEGREE], planet[count][MIN]);
			wattroff(win, COLOR_PAIR(WATER));
			break;
	}
}

void planet_pos(WINDOW *win, double sign_cusp[], double *planet[], int *zodiac[],
int radius, int cy, int cx, const char *pl_sym[])
{
	const int iter_count = 64;
	const int max_distance = 11;
	const double convergence_thresh = 0.1;
	const double str_base = 4.0;

	const int pcount = 12;
	double adjusted_pos[12] = {0};
	
	for (int i = 0; i < pcount; ++i)
		adjusted_pos[i] = planet[i][LONG];
		
	for (int iter = 0; iter < iter_count; ++iter)
	{
		double max_change = 0.0;
		
		for (int i = 0; i < pcount; ++i)
		{
			double current = adjusted_pos[i];
			double degree_offset = 0.0;
			
			for (int j = 0; j < pcount; ++j)
			{
				if (i != j)
				{
					double signed_distance = adjusted_pos[j] - current;
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
						
						degree_offset += str_base * strength * place;
					}
				}
			}
			double new_long = current + degree_offset;
			
			while (new_long < 0.0)
				new_long += 360.0;
			while (new_long >= 360.0)
				new_long -= 360.0;
				
			double change = fabs(new_long - current);
			max_change = (change > max_change) ? change : max_change;
			
			adjusted_pos[i] = new_long;
		}
		if (max_change < convergence_thresh)
			break;
	}
	
	for (int i = 0; i < pcount; ++i)
	{
		int zo_sign = (int)(sign_cusp[1] / 30.0);
		double as_sign = zo_sign * 30.0;
	
		double zo_pos_radian = (adjusted_pos[i] - as_sign) * M_PI / 180;
		double cos_rad = cos(zo_pos_radian);
		double sin_rad = sin(zo_pos_radian);
		
		int sym_len = (int)strlen(pl_sym[i]);
		int x = (cx - (int)(radius * cos_rad)) - sym_len / 2;
		int y = cy + (int)(radius * sin_rad * 0.5);
		
		degree_color(win, y-1, x, i, planet, zodiac);
		mvwaddstr(win, y, x, pl_sym[i]);
	
		if (planet[i][RETRO] > 0 && i != SE_TRUE_NODE)
		{
			wattron(win, COLOR_PAIR(FIRE));
			mvwprintw(win, y, x-1, "r");
			wattroff(win, COLOR_PAIR(FIRE));
		}
			
		if ((int)planet[i][STATION] == STATION_R)
		{
			wattron(win, COLOR_PAIR(EARTH));
			mvwaddstr(win, y, x-2, "sr");
			wattroff(win, COLOR_PAIR(EARTH));
		}
		else if ((int)planet[i][STATION] == STATION_D)
		{
			wattron(win, COLOR_PAIR(EARTH));
			mvwaddstr(win, y, x-2, "sd");
			wattroff(win, COLOR_PAIR(EARTH));
		}
	}
}

void ascmc_pos(WINDOW *win, double sign_cusp[], double *planet[], int *zodiac[],
int radius, int cy, int cx)
{
	const char *ascmc_sym[] = {"as", "mc", "ds", "ic"};
	
	int j = 0;
	int i = 12;
	for (j = 0; i < 16; ++i, ++j)
	{
		double rad = (planet[i][LONG] - sign_cusp[1]) * M_PI / 180.0;
		
		int x = cx - (int)(radius * cos(rad));
		int y = cy + (int)(radius * sin(rad) * 0.5);
		
		mvwaddstr(win, y, x, ascmc_sym[j]);
		
		degree_color(win, y-1, x, i, planet, zodiac);
	}
}

void zo_pos(WINDOW *win, double sign_cusp[],
int radius, int cy, int cx,
struct pxx *pxx, const char *zo_sym[], int *zodiac[])
{
	int asc_sign = (int)(pxx->dasc[LONG] / 30) - 1;
	for (int i = ARI; i < ZMAX; ++i)
	{
		int sign = ((i + asc_sign) % 12);
		if (sign == 0)
			sign = 12;

		int sign_inc = (((int)pxx->dasc[LONG] / 30) * 30) + 15;
		
		double rad = (sign_cusp[i] - sign_inc) * M_PI / 180.0;
		
		int x = cx - (int)(radius * cos(rad));
		int y = cy + (int)(radius * sin(rad) * 0.5);
		
		zo_color(win, y, x, sign, zo_sym, zodiac);
	}
}

void draw_house(WINDOW *win, double cusp[],
int radius, int cy, int cx,
chtype ch)
{
	for (int i = ARI; i < ZMAX; ++i)
	{
		double rad = cusp[i] * M_PI / 180.0;
		
		int edge_x = cx - (int)(radius * cos(rad));
		int edge_y = cy + (int)(radius * sin(rad) * 0.5);
		
		int half_x = cx - (int)((radius / 2) * cos(rad));
		int half_y = cy + (int)((radius / 2)  * sin(rad) * 0.5);
		
		int dx = edge_x - half_x;
		int dy = edge_y - half_y;
		
		int distance = (int)sqrt(dx * dx + dy * dy);
		if (distance == 0)
			distance = 1;
		
		for(int j = 0; j <= distance; j++)
		{
			int x = half_x + (dx * j) / distance;
			int y = half_y + (dy * j) / distance;
			mvwaddch(win, y, x, ch);
		}
	}
}

void draw_circle(WINDOW *win,
int radius, int cy, int cx,
chtype ch)
{
	const double ys = 0.5;
	const int step = radius * 12;
	
	for (int i = 0; i < step; ++i)
	{
		double angle = 2.0 * M_PI * i / step;
		
		int x = cx + (int)lround(radius * cos(angle));
		int y = cy + (int)lround(radius * sin(angle) * ys);
		
		mvwaddch(win, y, x, ch);
	}
}
