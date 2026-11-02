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

#define F_SIGN 0
#define F_PL0 1
#define F_PL1 2
#define F_LR 3
#define F_HR 4
#define F_ORB 5
#define F_FIND 6
#define F_NULL 7
#define F_MAX F_NULL+1

static void sign_long(struct ui *ui, char *str, int *du)
{
	for (int i = 0; i < 12; ++i)
	{
		if (strcmp(str, ui->sym.zo_sym[i]) == 0 || strcmp(str, ui->sym.zo_fname[i]) == 0)
		{
			du[0] = i*30;
			du[1] = (i+1)*30;
		}
	}
}

static void pl_name(struct ui *ui, char *p)
{
	for (int i = 0; i < 12; ++i)
	{
		if (strcmp(p, ui->sym.pl_name[i]) == 0 || strcmp(p, ui->sym.pl_fname[i]) == 0)
			snprintf(p, strlen(p)+1, "%d", i);
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

static void fill_tmp(struct tmp *dst, struct cdata *src)
{
	dst->year = src->year;
	dst->mon = src->mon;
	dst->mday = src->mday;
	dst->hour = src->hour;
	dst->min = src->min;
	dst->sec = src->sec;
}

static void fill_cdata(struct cdata *dst, struct tmp *tmp)
{
	dst->year = tmp->year;
	dst->mon = tmp->mon;
	dst->mday = tmp->mday;
	dst->hour = tmp->hour;
	dst->min = tmp->min;
	dst->sec = tmp->sec;
}

static void progress_bar(WINDOW *win, struct cdata *cdata, int r0, int r1)
{
	double whole = r1 - r0;
	double part = r1 - cdata->year;
	double percent = (part/whole) * 100;
	
	char *dance[10] = { "<(^_^)>", "<(^.^<)~", "~(>^.^)>", "~(>^.^<)", "<(0.0)>"};
	
	int rp = 100 - percent;
	
	int star = 0;
	if (rp > 0)
		star = rp / 10;
		
	for(int i = 0; i <= star; ++i)
	{
		srand(part);
		int d = rand() % 5;
		mvwprintw(win, 2, 3, "        ");
		mvwprintw(win, 2, 3, "%s", dance[d]);
		mvwprintw(win, 1, 0, "[");
		mvwprintw(win, 1, i+1, "|");
		mvwprintw(win, 1, 11, "]%d%%", rp);
		wrefresh(win);
	}
}

static double wrap_deg(double angle)
{
	angle = fmod(angle+180.0, 360);
	if (angle < 0.0)
		angle += 360.0;
	return angle - 180.0;
}

static void meantosouth(double *xx)
{
	xx[LONG] = (xx[LONG] + 180);
	if (xx[LONG] >= 360)
		xx[LONG] -= 360;
}

static int find_conj(WINDOW *win, struct cdata *cdata, struct tmp **tmp,
double d, double u, int p0, int p1, int r0, int r1, int orb, char **list)
{
	werase(win);
	wrefresh(win);
	
	struct tm temp = {0};
	time_t t = 0, last = 0;

	cpt(cdata, &temp, &t, IN);
	
	int iflag = SEFLG_SWIEPH | SEFLG_SPEED;
	double xx0[6], xx1[6];
	char serr[AS_MAXCH];
	double diff, diff_s;
	time_t step;
	int c = 0;
	
	const double arcsec = 1.0 / 3600.0;
	const double day_sec = 86400.0;
	const double max_angle = 0.5;
	const double max_step = day_sec * 24.0;
	const double min_step = 1.0;
	
	bool sn0 = FALSE;
	bool sn1 = FALSE;
	if (p0 == 10)
	{
		p0 = 11;
		sn0 = TRUE;
	}
	if (p1 == 10)
	{
		p1 = 11;
		sn1 = TRUE;
	}
		
	long iter = 0;
	for(;;)
	{
		calculate_utc(cdata);
	
		double jd_ut = swe_julday(cdata->utc_year, cdata->utc_mon, 
		cdata->utc_mday, cdata->utc_hour, SE_GREG_CAL);
	
		swe_calc_ut(jd_ut, p0, iflag, xx0, serr);
		swe_calc_ut(jd_ut, p1, iflag, xx1, serr);
		
		if (sn0 == TRUE)
			meantosouth(xx0);
		if (sn1 == TRUE)
			meantosouth(xx1);
	
		diff = wrap_deg(xx0[LONG] - xx1[LONG]);
		diff_s = xx0[LONG_S] - xx1[LONG_S];
		
		if (cdata->year < r0 || cdata->year > r1)
			return c;
	
		if (fabs(diff) <= arcsec+orb)
		{
			if (llabs((long long)(t - last)) > max_step && 
			xx0[LONG] < u && xx0[LONG] > d && xx1[LONG] < u && xx1[LONG] > d)
			{
				if (c >= MAXBUF)
					return c;
				list[c] = ecalloc(1, MAXBUF);
				tmp[c] = ecalloc(1, sizeof(*tmp[c]));
				fill_tmp(tmp[c], cdata);
				snprintf(list[c], MAXBUF, "%d.%d.%d ", cdata->year, cdata->mon, cdata->mday);
				c++;
				last = t;
			}
			
			t+= max_step;
			cpt(cdata, &temp, &t, OUT);
		}
		else
		{
			double correction_sec = 0.0;
			
			if (fabs(diff) < max_angle && fabs(diff_s) > 1e-9)
				correction_sec = -diff / diff_s * day_sec;
			
			if (correction_sec > 0.0 && correction_sec < max_step)
			{
				step = (time_t)llround(correction_sec);
				if (step < 1)
					step = 1;
				t+= step;
			}
			else
			{
			double step_sec;
				if (fabs(diff_s) < 1e-9)
					step_sec = max_step;
				else
					step_sec = max_angle / fabs(diff_s) * day_sec;
					
				if (step_sec > max_step)
					step_sec = max_step;
				if (step_sec < min_step)
					step_sec = min_step;
				step = (time_t)llround(step_sec);
				
				t+= step;
			}
			cpt(cdata, &temp, &t, OUT);
		}
		
		if (iter++ % 10000 == 0)
		{
			progress_bar(win, cdata, r0, r1);
			enanosleep(10);
		}
	}
	return -1;
}

void print_asp_menu(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac, 
WINDOW *win, WINDOW *subwin, struct tmp **tmp, char **list, int idx)
{
	ITEM **item = ecalloc(idx+1, sizeof(*item));
	
	for (int i = 0; i < idx; ++i)
		item[i] = new_item(list[i], NULL);
	item[idx] = NULL;

	MENU *menu = new_menu(item);	
	
	set_menu_win(menu, win);
	set_menu_sub(menu, subwin);
	
	set_menu_fore(menu, COLOR_PAIR(M_COLOR) | A_REVERSE);
	set_menu_back(menu, COLOR_PAIR(M_COLOR));
	menu_opts_off(menu, O_NONCYCLIC);
	
	post_menu(menu);
	fill_cdata(cdata, tmp[0]);
	new_chart(cdata, pxx, ui, planet, zodiac);
	doupdate();
	
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

void aspect_find(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac)
{
	char **list = ecalloc(MAXBUF, sizeof(*list));
	int height = 22;
	int width = 16;
	int sb = 1;
	int fl = 12;
	
	int sy = 3;
	int sx = (COLS - width);
	
	WINDOW *win = newwin(height, width, sy, sx);
	WINDOW *subwin = derwin(win, height - 2, width - 2, 0, 0);
	
	wbkgdset(win, COLOR_PAIR(M_COLOR));
	
	keypad(win, TRUE);	
	
	FIELD *field[F_MAX];
	field[F_SIGN] = new_field(1, fl, 2, 2, 0, 0);
	set_field_back(field[F_SIGN], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(field[F_SIGN], O_STATIC);
	field_opts_off(field[F_SIGN], O_AUTOSKIP);
	
	field[F_PL0] = new_field(1, fl, 5, 2, 0, 0);
	set_field_back(field[F_PL0], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(field[F_PL0], O_STATIC);
	field_opts_off(field[F_PL0], O_AUTOSKIP);
	
	field[F_PL1] = new_field(1, fl, 8, 2, 0, 0);
	set_field_back(field[F_PL1], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(field[F_PL1], O_STATIC);
	field_opts_off(field[F_PL1], O_AUTOSKIP);
	
	field[F_LR] = new_field(1, fl, 11, 2, 0, 0);
	set_field_back(field[F_LR], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(field[F_LR], O_STATIC);
	field_opts_off(field[F_LR], O_AUTOSKIP);
	
	field[F_HR] = new_field(1, fl, 14, 2, 0, 0);
	set_field_back(field[F_HR], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(field[F_HR], O_STATIC);
	field_opts_off(field[F_HR], O_AUTOSKIP);
	
	field[F_ORB] = new_field(1, fl, 17, 2, 0, 0);
	set_field_back(field[F_ORB], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(field[F_ORB], O_STATIC);
	field_opts_off(field[F_ORB], O_AUTOSKIP);
	set_field_buffer(field[F_ORB], 0, "0");
	
	field[F_FIND] = new_field(1, 8, 19, sb, 0, 0);
	field_opts_off(field[F_FIND], O_EDIT);
	
	field[F_NULL] = NULL;
	
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
	mvwprintw(win, 19, sb, "[ find ]");
	
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
				if (index == F_FIND)
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
		
		if (index == F_FIND)
		{
			wattron(win, A_REVERSE);
			mvwprintw(win, 19, sb, "[ find ]");
			wattroff(win, A_REVERSE);
		}
		else
			mvwprintw(win, 19, sb, "[ find ]");
			
		pos_form_cursor(form);
		box(win, 0, 0);
	
		wrefresh(win);
	}
	curs_set(0);
	
	int idx = 0;
	struct tmp **tmp = ecalloc(MAXBUF, sizeof(*tmp));
	
	if (!c)
	{
		char *sign = field_buffer(field[F_SIGN], 0);
		char *p0 = field_buffer(field[F_PL0], 0);
		char *p1 = field_buffer(field[F_PL1], 0);
		char *r0 = field_buffer(field[F_LR], 0);
		char *r1 = field_buffer(field[F_HR], 0);
		char *orb = field_buffer(field[F_ORB], 0);
		
		ftrim(field[F_SIGN], sign);
		ftrim(field[F_PL0], p0);
		ftrim(field[F_PL1], p1);
		ftrim(field[F_LR], r0);
		ftrim(field[F_HR], r1);
		ftrim(field[F_ORB], orb);
		
		int du[2] = {0};
		sign_long(ui, sign, du);
		pl_name(ui, p0);
		pl_name(ui, p1);
		
		cdata->year = atoi(r0);
		idx = find_conj(win, cdata, tmp, du[0], du[1], atoi(p0), atoi(p1), atoi(r0), atoi(r1), atoi(orb), list);
	}
			
	unpost_form(form);
	set_form_fields(form, NULL);
	for (int i = 0; i < F_MAX; ++i)
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
