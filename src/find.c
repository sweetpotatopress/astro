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
#include "swephexp.h"
#include "astro.h"
#include "ui.h"
#include "init.h"
#include "chronos.h"

void sign_long(struct ui *ui, char *str, int *du)
{
	for (int i = 0; i < 12; ++i)
	{
		if (strcmp(str, ui->sym.zo_sym[i]) == 0)
		{
			du[0] = i*30;
			du[1] = (i+1)*30;
		}
	}
}

struct tmp {
	int year;
	int mon;
	int mday;
	int hour;
	int min;
	int sec;
};

void fill_tmp(struct tmp *dst, struct cdata *src)
{
	int year = src->year;
	int mon = src->mon;
	int mday = src->mday;
	int hour = src->hour;
	int min = src->min;
	int sec = src->sec;
	
	dst->year = year;
	dst->mon = mon;
	dst->mday = mday;
	dst->hour = hour;
	dst->min = min;
	dst->sec = sec;
}

void fill_cdata(struct cdata *dst, struct tmp *tmp)
{
	dst->year = tmp->year;
	dst->mon = tmp->mon;
	dst->mday = tmp->mday;
	dst->hour = tmp->hour;
	dst->min = tmp->min;
	dst->sec = tmp->sec;
}

int find_conj(struct cdata *cdata, struct tmp **tmp, double d, double u, int p0, int p1, int r0, int r1, int orb, char **list)
{
	struct tm temp = {0};
	time_t t = 0;

	cpt(cdata, &temp, &t, IN);
	
	int iflag = SEFLG_SWIEPH;
	double xx0[6], xx1[6];
	char serr[AS_MAXCH];
	double diff;
	int i = 0;
	int ld[2] = {0};
	
	for(;;)
	{
		calculate_utc(cdata);
	
		double jd_ut = swe_julday(cdata->utc_year, cdata->utc_mon, 
		cdata->utc_mday, cdata->utc_hour, SE_GREG_CAL);
	
		swe_calc_ut(jd_ut, p0, iflag, xx0, serr);
		swe_calc_ut(jd_ut, p1, iflag, xx1, serr);
			
		diff = fabs(xx0[LONG] - xx1[LONG]);
		
		time_t step = 
			fabs(diff) > 48.0 ? 86400*24 :
			fabs(diff) > 24.0 ? 86400*12 :
			fabs(diff) > 12.0 ? 86400*6 :
			fabs(diff) > 6.0 ? 86400*3 :
			fabs(diff) > 2.0 ? 86400 :
			fabs(diff) > 0.1 ? 3600 :
			fabs(diff) > 0.002 ? 60 : 1;
			
		t+= step;
		
		cpt(cdata, &temp, &t, OUT);
		
		if (xx0[LONG] < u && xx0[LONG] > d && xx1[LONG] < u && xx1[LONG] > d)
		{
			if (fabs(diff) < orb + JUL_SEC)
			{
				if (cdata->year != ld[0] && cdata->mon != ld[1])
				{
					if (i >= MAXBUF)
						return i;
					list[i] = ecalloc(1, MAXBUF);
					tmp[i] = ecalloc(1, sizeof(*tmp[i]));
					fill_tmp(tmp[i], cdata);
					snprintf(list[i], MAXBUF, "%d.%d.%d ", cdata->year, cdata->mon, cdata->mday);
					i++;
					ld[0] = cdata->year;
					ld[1] = cdata->mon;
				}
			}
		}
		
		if (cdata->year < r0 || cdata->year > r1)
		{
			return i;
			break;
		}
	}
	return -1;
}

void print_asp_menu(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac, 
WINDOW *win, WINDOW *subwin, struct tmp **tmp, char **list, int idx)
{
	ITEM **item = ecalloc(idx+1, sizeof(*item));
	
	for (int i = 0; i < idx; ++i)
		item[i] = new_item(list[i], list[i]);
	item[idx] = NULL;

	MENU *menu = new_menu(item);	
	
	set_menu_win(menu, win);
	set_menu_sub(menu, subwin);
	
	set_menu_fore(menu, COLOR_PAIR(M_COLOR) | A_REVERSE);
	set_menu_back(menu, COLOR_PAIR(M_COLOR));
	menu_opts_off(menu, O_NONCYCLIC);
	
	post_menu(menu);
	
	ITEM *s = NULL;
	
	int ch, done = 0, iret = 0;
	while(!done && (ch = wgetch(win)))
	{
		switch(ch)
		{
			case 'j': case KEY_DOWN:
				menu_driver(menu, REQ_DOWN_ITEM);
				s = current_item(menu);
				iret = item_index(s);
				fill_cdata(cdata, tmp[iret]);
				
				new_chart(cdata, pxx, ui, planet, zodiac);
				doupdate();
				break;
			case 'k': case KEY_UP:
				menu_driver(menu, REQ_UP_ITEM);
				s = current_item(menu);
				iret = item_index(s);
				fill_cdata(cdata, tmp[iret]);
				
				new_chart(cdata, pxx, ui, planet, zodiac);
				doupdate();
	
				break;
			case 'l': case KEY_RIGHT:
				done = 1;
				break;
			case 'q': case 27:
				done = 1;
				break;
		}	
	}
	unpost_menu(menu);
	free_menu(menu);
	set_menu_items(menu, NULL);
	
	for (int i = 0; i <= idx; ++i)
		free_item(item[i]);
	free(item);
}

void aspect_search(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac)
{
	char **list = ecalloc(MAXBUF, sizeof(*list));
	int height = 25;
	int width = 30;
	
	int starty = (LINES- height) / 2;
	int startx = (COLS - width);
	
	WINDOW *win = newwin(height, width, starty, startx);
	WINDOW *subwin = derwin(win, height - 2, width - 2, 0, 0);
	
	wbkgdset(win, COLOR_PAIR(M_COLOR));
	
	keypad(win, TRUE);	
	
	FIELD *field[8];
	field[0] = new_field(1, 25, 2, 2, 0, 0);
	set_field_back(field[0], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(field[0], O_STATIC);
	field_opts_off(field[0], O_AUTOSKIP);
	
	field[1] = new_field(1, 25, 5, 2, 0, 0);
	set_field_back(field[1], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(field[1], O_STATIC);
	field_opts_off(field[1], O_AUTOSKIP);
	
	field[2] = new_field(1, 25, 8, 2, 0, 0);
	set_field_back(field[2], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(field[2], O_STATIC);
	field_opts_off(field[2], O_AUTOSKIP);
	
	field[3] = new_field(1, 25, 11, 2, 0, 0);
	set_field_back(field[3], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(field[3], O_STATIC);
	field_opts_off(field[3], O_AUTOSKIP);
	
	field[4] = new_field(1, 25, 14, 2, 0, 0);
	set_field_back(field[4], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(field[4], O_STATIC);
	field_opts_off(field[4], O_AUTOSKIP);
	
	field[5] = new_field(1, 25, 17, 2, 0, 0);
	set_field_back(field[5], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(field[5], O_STATIC);
	field_opts_off(field[5], O_AUTOSKIP);
	set_field_buffer(field[5], 0, "0");
	
	field[6] = new_field(1, 8, 20, 9, 0, 0);
	set_field_buffer(field[6], 0, "[ search ]");
	field_opts_off(field[6], O_EDIT);
	
	field[7] = NULL;
	
	FORM *form = new_form(field);
	set_form_win(form, win);
	set_form_sub(form, subwin);
	
	post_form(form);
	
	mvwaddstr(win, 1, 1, "sign");
	mvwaddstr(win, 4, 1, "planet");
	mvwaddstr(win, 7, 1, "planet");
	mvwaddstr(win, 10, 1, "low range");
	mvwaddstr(win, 13, 1, "hi range");
	mvwaddstr(win, 16, 1, "orb");
	box(win, 0, 0);
	
	set_current_field(form, field[0]);
	wrefresh(win);
	pos_form_cursor(form);
	curs_set(1);
	
	FIELD *current = NULL;
	int index = 0;
	
	int done = 0, ch = 0, c = 0;
	while(!done && (ch = wgetch(win)))
	{
		switch (ch)
		{
			case '\\': case '[': case ']':
				form_driver(form, REQ_VALIDATION);
				done = 1;
				break;	
	
			case '\n':
				if (index == 6)
				{
					form_driver(form, REQ_VALIDATION);
					done = 1;
					break;
				}
	
				form_driver(form, REQ_NEXT_FIELD);
				form_driver(form, REQ_END_LINE);
				break;
			case KEY_DOWN: case ';':
				form_driver(form, REQ_NEXT_FIELD);
				form_driver(form, REQ_END_LINE);
				break;
				
			case KEY_UP: case '\'':
				form_driver(form, REQ_PREV_FIELD);
				form_driver(form, REQ_END_LINE);
				break;
				
			case KEY_LEFT:
				form_driver(form, REQ_LEFT_CHAR);
				break;
				
			case KEY_RIGHT:
				form_driver(form, REQ_RIGHT_CHAR);
				break;
				
			case KEY_BACKSPACE:
				form_driver(form, REQ_DEL_PREV);
				break;
			case 27:
				c = 1;
				done = 1;
				break;
			default:
				form_driver(form, ch);
				break;
		}
		current = current_field(form);
		index = field_index(current);
		
		if (index == 6)
		{
			wattron(win, A_REVERSE);
			mvwprintw(win, 20, 9, "[ search ]");
			wattroff(win, A_REVERSE);
		}
		else
			mvwprintw(win, 20, 9, "[ search ]");
			
		pos_form_cursor(form);
		box(win, 0, 0);
	
		wrefresh(win);
	}
	int idx = 0;
	struct tmp **tmp = ecalloc(MAXBUF, sizeof(*tmp));
	
	if (!c)
	{
		char *sign = field_buffer(field[0], 0);
		char *p0 = field_buffer(field[1], 0);
		char *p1 = field_buffer(field[2], 0);
		char *r0 = field_buffer(field[3], 0);
		char *r1 = field_buffer(field[4], 0);
		char *orb = field_buffer(field[5], 0);
		
		ftrim(field[0], sign);
		ftrim(field[1], p0);
		ftrim(field[2], p1);
		ftrim(field[3], r0);
		ftrim(field[4], r1);
		ftrim(field[5], orb);
		
		int du[2] = {0};
		sign_long(ui, sign, du);
		
		cdata->year = atoi(r0);
		idx = find_conj(cdata, tmp, du[0], du[1], atoi(p0), atoi(p1), atoi(r0), atoi(r1), atoi(orb), list);
	}
	
	curs_set(0);
			
	unpost_form(form);
	set_form_fields(form, NULL);
	for (int i = 0; i < 7; ++i)
		free_field(field[i]);
	free_form(form);
	
	if (idx > 0)
	{
		werase(win);
		print_asp_menu(cdata, pxx, ui, planet, zodiac, win, subwin, tmp, list, idx);
	}
		
	werase(subwin);
	wrefresh(subwin);
	werase(win);
	wrefresh(win);
	delwin(subwin);
	delwin(win);
	
	for (int i = 0; i < idx; ++i)
		free(list[i]);
	free(list);
	for (int i = 0; i < idx; ++i)
		free(tmp[i]);
	free(tmp);
}
