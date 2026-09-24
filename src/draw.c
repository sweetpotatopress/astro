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
#include "ui.h"
#include "draw.h"
#include "chronos.h"

void zo_color(WINDOW *win, struct ui *ui, int y, int x, int sign, int *zodiac[])
{
	switch(zodiac[sign][ELEMENT])
	{
		case FIRE:
			wattron(win, COLOR_PAIR(FIRE));
			mvwaddstr(win, y, x, ui->sym.zo_sym[sign]);
			wattroff(win, COLOR_PAIR(FIRE));
			break;
		case EARTH:
			wattron(win, COLOR_PAIR(EARTH));
			mvwaddstr(win, y, x, ui->sym.zo_sym[sign]);
			wattroff(win, COLOR_PAIR(EARTH));
			break;
		case AIR:
			wattron(win, COLOR_PAIR(AIR));
			mvwaddstr(win, y, x, ui->sym.zo_sym[sign]);
			wattroff(win, COLOR_PAIR(AIR));
			break;
		case WATER:
			wattron(win, COLOR_PAIR(WATER));
			mvwaddstr(win, y, x, ui->sym.zo_sym[sign]);
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

void planet_pos(WINDOW *win, struct cdata *cdata, struct ui *ui, double **planet, int **zodiac)
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
		int zo_sign;
		if (ui->cc == TRANSIT)
			zo_sign = (int)(cdata->t_cusp / 30.0);
		else
			zo_sign = (int)(cdata->sign_cusp[1] / 30.0);
		double as_sign = zo_sign * 30.0;
	
		double zo_pos_radian = (adjusted_pos[i] - as_sign) * M_PI / 180;
		double cos_rad = cos(zo_pos_radian);
		double sin_rad = sin(zo_pos_radian);
		
		int sym_len = (int)strlen(ui->sym.pl_sym[i]);
		int x = (ui->cx - (int)(ui->pr * cos_rad)) - sym_len / 2;
		int y = ui->cy + (int)(ui->pr * sin_rad * 0.5);
		
		int x_in = (ui->cx - (int)((ui->ir-1) * cos_rad));
		int y_in = ui->cy + (int)((ui->ir-1) * sin_rad * 0.5);
		
		int x_in_m = (ui->cx - (int)((ui->ir+1) * cos_rad));
		int y_in_m = ui->cy + (int)((ui->ir+1) * sin_rad * 0.5);
	
		planet[i][PL_X] = x_in;
		planet[i][PL_Y] = y_in;
		
		degree_color(win, y-1, x, i, planet, zodiac);
		mvwaddstr(win, y, x, ui->sym.pl_sym[i]);
		
		if (i <= SE_PLUTO)
			mvwaddch(win, y_in_m, x_in_m, '+');
	
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

void ascmc_pos(WINDOW *win, struct cdata *cdata, struct ui *ui, double *planet[], int *zodiac[])
{
	const char *ascmc_sym[] = {"as", "mc", "ds", "ic"};
	
	int j = 0;
	int i = 12;
	for (j = 0; i < 16; ++i, ++j)
	{
		double rad = (planet[i][LONG] - cdata->sign_cusp[1]) * M_PI / 180.0;
		
		int x = ui->cx - (int)(ui->ar * cos(rad));
		int y = ui->cy + (int)(ui->ar * sin(rad) * 0.5);
		
		mvwaddstr(win, y, x, ascmc_sym[j]);
		
		degree_color(win, y-1, x-1, i, planet, zodiac);
	}
}

void zo_pos(WINDOW *win, struct cdata *cdata, struct pxx *pxx, struct ui *ui, int **zodiac)
{
	int asc_sign = (int)(pxx->dasc[LONG] / 30) - 1;
	for (int i = ARI; i < ZMAX; ++i)
	{
		int sign = ((i + asc_sign) % 12);
		if (sign == 0)
			sign = 12;

		int sign_inc = (((int)pxx->dasc[LONG] / 30) * 30) + 15;
		
		double rad = (cdata->sign_cusp[i] - sign_inc) * M_PI / 180.0;
		
		int x = ui->cx - (int)(ui->zr * cos(rad));
		int y = ui->cy + (int)(ui->zr * sin(rad) * 0.5);
		
		zo_color(win, ui, y, x, sign, zodiac);
	}
}

void draw_house(WINDOW *win, struct cdata *cdata, struct ui *ui, chtype ch)
{
	for (int i = ARI; i < ZMAX; ++i)
	{
		double rad = cdata->cusp[i] * M_PI / 180.0;
		
		int edge_x = ui->cx - (int)(ui->hr * cos(rad));
		int edge_y = ui->cy + (int)(ui->hr * sin(rad) * 0.5);
		
		int half_x = ui->cx - (int)((ui->hr / 2) * cos(rad));
		int half_y = ui->cy + (int)((ui->hr / 2) * sin(rad) * 0.5);
		
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

void draw_circle(WINDOW *win, struct ui *ui, int radius, chtype ch)
{
	const double ys = 0.5;
	const int step = ui->radius * 12;
	
	for (int i = 0; i < step; ++i)
	{
		double angle = 2.0 * M_PI * i / step;
		
		int x = ui->cx + (int)lround(radius * cos(angle));
		int y = ui->cy + (int)lround(radius * sin(angle) * ys);
		
		mvwaddch(win, y, x, ch);
	}
}

static void draw_line(WINDOW *win, int y0, int x0, int y1, int x1, chtype ch)
{
	int dx = abs(x1 - x0);
	int sx = (x0 < x1) ? 1 : -1;
	int dy = -abs(y1 - y0);
	int sy = (y0 < y1) ? 1 : -1;
	int err = dx + dy;
	
	for (;;) 
	{
		mvwaddch(win, y0, x0, ch);
		
		if (x0 == x1 && y0 == y1)
			break;
		
		int e2 = 2 * err;
		
		if (e2 >= dy)
		{
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx)
		{
			err += dx;
			y0 += sy;
		}
	}
}

static chtype line_char(int y0, int x0, int y1, int x1)
{
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	
	if (dx > dy * 2)
		return '`';
	if (dy > dx * 2)
		return '`';
	if ((x1 - x0) * (y1 - y0) < 0)
		return '.';
		
	return '.';
}

void draw_aspect(WINDOW *win, double **planet)
{
	double sextile = 60.0;
	double square = 90.0;
	double trine = 120.0;
	double opposition = 180.0;
	
	for (int i = 0; i <= SE_PLUTO; ++i)
	{
		for (int j = i+1; j <= SE_PLUTO; ++j)
		{
			double diff = fmod(fabs(planet[i][LONG] - planet[j][LONG]), 360);
			if (diff > 180)
				diff = 360.0 - diff;
			double house_diff = fabs(planet[i][DEGREE] - planet[j][DEGREE]);
			
			chtype ch = line_char((int)planet[i][PL_Y], (int)planet[i][PL_X],
				(int)planet[j][PL_Y], (int)planet[j][PL_X]);
				
			if (planet[i][LONG_S] > planet[j][LONG_S] && planet[i][DEGREE] <= planet[j][DEGREE])
				ch = '+';
			
			if (fabs(diff - sextile) <= 7.0 && house_diff <= 7)
			{
				wattron(win, COLOR_PAIR(EARTH));
				draw_line(win, (int)planet[i][PL_Y], (int)planet[i][PL_X],
					(int)planet[j][PL_Y], (int)planet[j][PL_X], ch);
				wattroff(win, COLOR_PAIR(EARTH));
			}
			if (fabs(diff - square) <= 7.0 && house_diff <= 7)
			{
				wattron(win, COLOR_PAIR(FIRE));
				draw_line(win, (int)planet[i][PL_Y], (int)planet[i][PL_X],
					(int)planet[j][PL_Y], (int)planet[j][PL_X], ch);
				wattroff(win, COLOR_PAIR(FIRE));
			}
			if (fabs(diff - trine) <= 7.0 && house_diff <= 7)
			{
				wattron(win, COLOR_PAIR(WATER));
				draw_line(win, (int)planet[i][PL_Y], (int)planet[i][PL_X],
					(int)planet[j][PL_Y], (int)planet[j][PL_X], ch);
				wattroff(win, COLOR_PAIR(WATER));
			}
			if (fabs(diff - opposition) <= 7.0 && house_diff <= 7)
			{
				wattron(win, COLOR_PAIR(FIRE));
				draw_line(win, (int)planet[i][PL_Y], (int)planet[i][PL_X],
					(int)planet[j][PL_Y], (int)planet[j][PL_X], ch);
				wattroff(win, COLOR_PAIR(FIRE));
			}
		}
	}
}
		
