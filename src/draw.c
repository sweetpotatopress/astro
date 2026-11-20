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
	int sign = (int)planet[count][SIGN];
	
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

void string_color(WINDOW *win, int y, int x, char *str, int color)
{
	wattron(win, COLOR_PAIR(color));
	mvwprintw(win, y, x, "%s", str);
	wattroff(win, COLOR_PAIR(color));
}

void planet_pos(WINDOW *win, struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac)
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
			
			norm360(&new_long);
				
			double change = fabs(new_long - current);
			max_change = (change > max_change) ? change : max_change;
			
			adjusted_pos[i] = new_long;
		}
		if (max_change < convergence_thresh)
			break;
	}
	
	for (int i = 0; i < pcount; ++i)
	{
		int zo_sign = pxx->dasc[LONG];
		
		if (cdata->hsys == 'W')
			zo_sign = floor(cdata->sign_cusp[0] / 30.0) * 30;
		if (cdata->hsys == 'N')
			zo_sign = cdata->cusp[0];
		if (ui->cc == TRANSIT)
			zo_sign = cdata->t_cusp;
	
		double zo_pos_radian = (adjusted_pos[i] - zo_sign) * M_PI / 180;
		double cos_rad = cos(zo_pos_radian);
		double sin_rad = sin(zo_pos_radian);
		
		int sym_len = (int)strlen(ui->sym.pl_sym[i]);
		int pl_x = (ui->cx - (int)(ui->pr * cos_rad)) - sym_len / 2;
		int x = (ui->cx - (int)(ui->pr * cos_rad));
		int y = ui->cy + (int)(ui->pr * sin_rad * 0.5);
		
		int x_in = (ui->cx - (int)((ui->ir-1) * cos_rad));
		int y_in = ui->cy + (int)((ui->ir-1) * sin_rad * 0.5);
		
		int x_in_m = (ui->cx - (int)((ui->ir+1) * cos_rad));
		int y_in_m = ui->cy + (int)((ui->ir+1) * sin_rad * 0.5);
	
		planet[i][PL_X] = x_in;
		planet[i][PL_Y] = y_in;
		
		degree_color(win, y-1, x-2, i, planet, zodiac);
		mvwaddstr(win, y, pl_x, ui->sym.pl_sym[i]);
		
		if (i <= SE_PLUTO && ui->cc != TRANSIT && ui->cc != SYNASTRY && !ui->aspect_trig)
			mvwaddch(win, y_in_m, x_in_m, '+');
	
		if (planet[i][RETRO] > 0 && i != SE_TRUE_NODE)
			string_color(win, y, pl_x-1, "r", FIRE);
			
		if ((int)planet[i][STATION] == STATION_R)
			string_color(win, y, pl_x-2, "sr", EARTH);
			
		else if ((int)planet[i][STATION] == STATION_D)
			string_color(win, y, pl_x-2, "sd", EARTH);
	}
}

static void ascmc_pos(WINDOW *win, struct cdata *cdata, struct ui *ui, double *planet[], int *zodiac[])
{
	const char *ascmc_sym[] = {"as", "mc", "ds", "ic"};
	
	int j = 0;
	int i = 12;
	for (j = 0; i < 16; ++i, ++j)
	{
		double rad = (planet[i][LONG] - cdata->cusp[0]) * M_PI / 180.0;
		
		int x = ui->cx - (int)(ui->ar * cos(rad));
		int y = ui->cy + (int)(ui->ar * sin(rad) * 0.5);
		
		mvwaddstr(win, y, x, ascmc_sym[j]);
		
		degree_color(win, y-1, x-1, i, planet, zodiac);
	}
}

static void zo_pos(WINDOW *win, struct cdata *cdata, struct pxx *pxx, struct ui *ui, int **zodiac)
{
	int asc_sign = (int)pxx->dasc[SIGN];
	for (int i = ARI; i < ZMAX; ++i)
	{
		int sign = ((i + asc_sign) % 12);

		double rad = (cdata->sign_cusp[i] + 15 - cdata->cusp[0]) * M_PI / 180.0;
		
		int x = ui->cx - (int)(ui->zr * cos(rad));
		int y = ui->cy + (int)(ui->zr * sin(rad) * 0.5);
		
		zo_color(win, ui, y, x, sign, zodiac);
	}
}

static void draw_zo_line(WINDOW *win, struct cdata * cdata, struct ui *ui, chtype ch)
{
	for (int i = ARI; i < ZMAX; ++i)
	{
		double rad = (cdata->sign_cusp[i] - cdata->cusp[0]) * M_PI / 180.0;
		
		int edge_x = ui->cx - (int)(ui->or * cos(rad));
		int edge_y = ui->cy + (int)(ui->or * sin(rad) * 0.5);
		
		int half_x = ui->cx - (int)((ui->zr-2) * cos(rad));
		int half_y = ui->cy + (int)((ui->zr-2) * sin(rad) * 0.5);
		
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

static const int sign_ele[12] = { 
	FIRE, EARTH, AIR, WATER,
	FIRE, EARTH, AIR, WATER,
	FIRE, EARTH, AIR, WATER };

static void draw_zo_color(WINDOW *win, struct cdata *cdata, struct ui *ui, int radius, chtype ch)
{
	const double pi = acos(-1.0);
	const int steps = 3600;

	for (int i = 0; i < steps; i++)
	{
		double deg = 360.0 * i / steps;
		double rad = deg * pi / 180.0;
		
		int x = ui->cx - (int)(radius * cos(rad));
		int y = ui->cy + (int)(radius * sin(rad) * 0.5);
		
		double longitude = fmod(deg + cdata->cusp[0], 360.0);
		norm360(&longitude);
			
		int sign = (int)(longitude / 30);
		if (sign > 11)
			sign = 11;
		
		wattron(win, COLOR_PAIR(sign_ele[sign]));
		mvwaddch(win, y, x, ch);
		wattroff(win, COLOR_PAIR(sign_ele[sign]));
	}
}

int house_menu(struct ui *ui)
{
	int count = 11;
	int ret = 87;
	
	int hsys[] = {
	'W', 'A', 'O', 'P',
	'C', 'K', 'R', 'B',
	'F', 'T', 'N' };
		
	char *head = "house system";
		
	int header = 4;
	int width = 20;
	int height = 11 + header;
	
	int sy = (LINES - height) / 2;
	int sx = (COLS - width);
	
	WINDOW *win = newwin(height, width, sy, sx);
	WINDOW *subwin = derwin(win, height - header, width - 2, 3, 1);
	ITEM **item = ecalloc(count+1, sizeof(ITEM *));
	
	for (int i = 0; i < count; i++)
		item[i] = new_item(ui->sym.hname[i], NULL);
	item[count] = NULL;
	
	MENU *menu = new_menu(item);
	
	box(win, 0, 0);
	keypad(win, TRUE);
	wbkgdset(win, COLOR_PAIR(M_COLOR));
	
	menu_opts_off(menu, O_NONCYCLIC);
	menu_opts_off(menu, O_SHOWDESC);
	set_menu_fore(menu, COLOR_PAIR(M_COLOR) | A_REVERSE);
	set_menu_back(menu, COLOR_PAIR(M_COLOR));
	set_menu_win(menu, win);
	set_menu_sub(menu, subwin);
	
	mvwprintw(win, 1, 1, " %s", head);
	mvwprintw(win, 1, width - 4, "{h}");
	mvwhline(win, 2, 1, ACS_HLINE, width - 2);

	post_menu(menu);
	wrefresh(win);
	
	ITEM *cur = NULL;
	
	int ch = 0, menu_done = 0;
	while (!menu_done && (ch = wgetch(win)))
	{
		cur = current_item(menu);
		
		switch(ch)
		{
			case 'j': case KEY_DOWN:
				menu_driver(menu, REQ_DOWN_ITEM);
				break;
			case 'k': case KEY_UP:
				menu_driver(menu, REQ_UP_ITEM);
				break;
			case 'l': case KEY_RIGHT: case '\n':
				ret = hsys[item_index(cur)];
				menu_done = 1;
				break;
			case 'q': case 27:
				menu_done = 1;
				break;
		}
			
		box(win, 0, 0);
	}
	
	unpost_menu(menu);
	free_menu(menu);
	
	for (int i = 0; i < count+1; ++i)
		free_item(item[i]);
	free(item);
	
	werase(win);
	wrefresh(win);
	delwin(subwin);
	delwin(win);
	
	return ret; 
}

struct house_deg {
	int sign;
	int deg;
	int min;
	int y;
	int x;
};

static void draw_house(WINDOW *win, struct cdata *cdata, struct ui *ui, chtype ch, struct house_deg **hd)
{
	for (int i = ARI; i < ZMAX; ++i)
	{
		double rad = (cdata->cusp[i] - cdata->cusp[0]) * M_PI / 180.0;
		
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
			if (j == 4)
			{
				hd[i]->sign = (int)cdata->cusp[i] / 30;
				hd[i]->deg = (int)cdata->cusp[i] % 30;
				hd[i]->min = (int)((cdata->cusp[i] - (int)cdata->cusp[i]) * 60);
				hd[i]->y = y;
				hd[i]->x = x;
			}
		}
	}
}	

static void draw_hd(WINDOW *win, struct house_deg **hd)
{
	for (int i = ARI; i < ZMAX; ++i)
	{
		if (i != 0 && i != 3 && i != 6 && i != 9)
		{
			wattron(win, COLOR_PAIR(sign_ele[hd[i]->sign]));
			mvwprintw(win, hd[i]->y, hd[i]->x, "%d*%02d", hd[i]->deg, hd[i]->min);
			wattroff(win, COLOR_PAIR(sign_ele[hd[i]->sign]));
		}
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

static void draw_aspect(WINDOW *win, double **planet)
{
	const double sextile = 60.0;
	const double square = 90.0;
	const double trine = 120.0;
	const double opposition = 180.0;
	
	const double aspects[4] = { sextile, square, trine, opposition };
	const int aspect_color[4] = { EARTH, FIRE, WATER, FIRE };
	
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
				
			if ((planet[i][LONG_S] > planet[j][LONG_S] && planet[i][DEGREE] <= planet[j][DEGREE]) ||
			(planet[j][LONG_S] > planet[i][LONG_S] && planet[j][DEGREE] <= planet[i][DEGREE]))
				ch = '+';
			
			for (int c = 0; c < 4; ++c)
			{
				if (fabs(diff - aspects[c]) <= 7.0 && house_diff <= 7)
				{
					wattron(win, COLOR_PAIR(aspect_color[c]));
					draw_line(win, (int)planet[i][PL_Y], (int)planet[i][PL_X],
						(int)planet[j][PL_Y], (int)planet[j][PL_X], ch);
					wattroff(win, COLOR_PAIR(aspect_color[c]));
				}
			}
	
		}
	}
}

static void draw_circle(WINDOW *win, struct ui *ui, int radius, chtype ch)
{
	int x = 0;
	int y = radius;
	int d = 3 -2 * radius;
	
	while (x <= y)
	{
		mvwaddch(win, ui->cy + y / 2, ui->cx + x, ch);
		mvwaddch(win, ui->cy + y / 2, ui->cx - x, ch);
		mvwaddch(win, ui->cy - y / 2, ui->cx + x, ch);
		mvwaddch(win, ui->cy - y / 2, ui->cx - x, ch);
		
		mvwaddch(win, ui->cy + x / 2, ui->cx + y, ch);
		mvwaddch(win, ui->cy + x / 2, ui->cx - y, ch);
		mvwaddch(win, ui->cy - x / 2, ui->cx + y, ch);
		mvwaddch(win, ui->cy - x / 2, ui->cx - y, ch);
	
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
		
void draw_chart(WINDOW *win, struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac)
{
	curs_set(0);
	werase(win);
	
	draw_zo_color(win, cdata, ui, ui->or, '`');
	draw_zo_color(win, cdata, ui, ui->radius, '`');
	draw_circle(win, ui, ui->ir, '`');
	
	struct house_deg **hd = ecalloc(ZMAX, sizeof(*hd));
	for (int i = 0; i < ZMAX; ++i)
		hd[i] = ecalloc(1, sizeof(*hd[i]));
		
	draw_house(win, cdata, ui, '.', hd);
	
	zo_pos(win, cdata, pxx, ui, zodiac);
	draw_zo_line(win, cdata, ui, '.');
	
	planet_pos(win, cdata, pxx, ui, planet, zodiac);
	
	if (!ui->aspect_trig)
		draw_aspect(win, planet);
	
	if (fabs(cdata->dlat) > 1e-6 && cdata->hsys != 'N')
		ascmc_pos(win, cdata, ui, planet, zodiac);
	
	if (cdata->hsys != 'W' && cdata->hsys != 'N')
		draw_hd(win, hd);
	
	// status bar
	const int bar_end = 20;
	mvwhline(win, 1, ui->win_w - bar_end, '-', COLS);
	mvwvline(win, 0, ui->win_w - bar_end, ':', 1);
	
	mvwprintw(win, 0, ui->win_w - (bar_end - 2), "%d :", ui->cc);
	if (cdata->dt == TRUE)
	{
		wattron(win, COLOR_PAIR(WATER));
		mvwprintw(win, 0, ui->win_w - (bar_end + 2), "d");
		wattroff(win, COLOR_PAIR(WATER));
	}
	for (int i = 0; i < ZMAX; ++i)
		free(hd[i]);
	free(hd);
}
