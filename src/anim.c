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

#include <time.h>
#include <form.h>
#include <errno.h>
#include "swephexp.h"
#include "astro.h"
#include "chronos.h"
#include "anim.h"
#include "draw.h"
#include "table.h"
#include "ui.h"
#include "init.h"

#define SECOND 6
#define MINUTE 5
#define HOUR 4
#define DAY 3
#define MONTH 2
#define YEAR 1

static void enanosleep(unsigned int ms)
{
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	nanosleep(&ts, NULL);
}

void cc_data(WINDOW *win, struct cdata *cdata, struct ui *ui)
{	
	int starty, startx;
	int count = (ui->cc == TRANSIT) ? 2 : 1;
	for (int i = 0; i < count; ++i)
	{
		if (ui->cc == TRANSIT)
		{
			i++;
			starty = 2;
			startx = COLS - 20;
		}
		else
		{
			starty = ui->cy - 4;
			startx = ui->cx - 10;
		}
		
		if(cdata->chart_name)
			mvwprintw(win, starty, startx, "%s", cdata->chart_name);
		
		if (ui->cc != TRANSIT)
		{
			starty += 1;
			if (cdata->state && !isdigit((unsigned char)cdata->state[0]) && strlen(cdata->state) > 1)
				mvwprintw(win, starty, startx, "%.22s, %s, %s", cdata->city, cdata->state, cdata->country);
			
			else if (cdata->country && cdata->city && strlen(cdata->country) > 0 && strlen(cdata->city) > 0)
				mvwprintw(win, starty, startx, "%.22s, %s", cdata->city, cdata->country);
		}
			
		starty += 1;
		
		const char *weekday[] = 
		{ "sun", "mon", "tue", "wed", "thu", "fri", "sat" };
		
		if(cdata->year && cdata->mon && cdata->mday)
			mvwprintw(win, starty, startx, "%s.%02d.%02d, %s", 
			ui->sym.month[cdata->mon], cdata->mday, cdata->year, weekday[cdata->wday]);
			
		starty += 1;
		if (cdata->hour >= 0)
		{
			int hour = cdata->hour;
			if (hour == 12)
				mvwprintw(win, starty, startx, "%02d:%02d:%02dPM", cdata->hour, cdata->min, cdata->sec);
			else if (hour > 12)	
				mvwprintw(win, starty, startx, "%02d:%02d:%02dPM", cdata->hour - 12, cdata->min, cdata->sec);
			else if (hour == 0)
				mvwprintw(win, starty, startx, "12:%02d:%02dAM", cdata->min, cdata->sec);
			else if (hour > 0 && hour < 12)
				mvwprintw(win, starty, startx, "%02d:%02d:%02dAM", cdata->hour, cdata->min, cdata->sec);
		}
		starty += 1;
		int utc;
		
		if ((int)cdata->utc_hour == 0)
			utc = cdata->hour - 24;
		else
			utc = cdata->hour - (int)cdata->utc_hour;
		if (utc > 14)
			utc -= 24;
		if (utc < - 12)
			utc += 24;
			
		if (cdata->isdst == YDST)
			mvwprintw(win, starty, startx, "DST UTC%+02d", utc);
		else
			mvwprintw(win, starty, startx, "UTC%+02d", utc);
		if (i > 0)
			return;
			
		starty += 1;
		if (cdata->timezone)
			mvwprintw(win, starty, startx, "%.30s", cdata->timezone);
		
		starty += 1;
		if (fabs(cdata->dlat) > 1e-6)
			mvwprintw(win, starty, startx, "%f", cdata->dlat);
		
		starty += 1;
		if (fabs(cdata->dlon) > 1e-6)
			mvwprintw(win, starty, startx, "%f", cdata->dlon);
	}
}

void realtime_chart(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac)
{
	nodelay(ui->main_win, TRUE);
		
	int ch = 0;
	while ((ch = wgetch(ui->main_win)) != 9)
	{
		set_localtime(cdata);
		wheel_init(ui->main_win, ui, 0, 0, 0);
		new_chart(cdata, pxx, ui, planet, zodiac);
		
		wattron(ui->main_win, COLOR_PAIR(FIRE));
		mvwprintw(ui->main_win, 0, COLS - 14, "*live");
		wattroff(ui->main_win, COLOR_PAIR(FIRE));
		
		wnoutrefresh(ui->main_win);
	
		for (int i = 0; i < 10; ++i)
		{
			enanosleep(10);
			if (ch == 9 || ch == 'q')
				break;
			table_trigger(ui, ch);
		}
		
		if (ch == 9 || ch == 'q')
			break;
	}
	wmove(ui->main_win, 0, COLS - 14);
	wclrtoeol(ui->main_win);
	nodelay(ui->main_win, FALSE);
}

void transit(struct cdata **cdata, struct pxx **pxx, struct ui *ui, double **planet, int **zodiac)
{
	ui->transit_window = newwin(LINES, COLS, 0, 0);
	ui->transit_panel = new_panel(ui->transit_window);
	
	ui->bcc = ui->cc;
	cdata[TRANSIT]->t_cusp = cdata[ui->bcc]->sign_cusp[1];
	
	wheel_init(ui->main_win, ui, 3, 0, 0);
	new_chart(cdata[ui->bcc], pxx[ui->bcc], ui, planet, zodiac);
	
	ui->cc = TRANSIT;
	planet_init(planet, ui->cc, pxx);
	pxx_init(cdata[ui->cc], pxx[ui->cc], planet);
	
	doupdate();
	animate_chart(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
				
	ui->cc = ui->bcc;
	wheel_init(ui->main_win, ui, 0, 0, 0);
	
	del_panel(ui->transit_panel);
	delwin(ui->transit_window);
}

void synastry(struct cdata **cdata, struct pxx **pxx, struct ui *ui, double **planet, int **zodiac, int key)
{
	wheel_init(ui->main_win, ui, 3, 0, 0);
	new_chart(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
	
	planet_init(planet, key, pxx);
	pxx_init(cdata[key], pxx[key], planet);
	
	double tmp = cdata[key]->sign_cusp[1];
	cdata[key]->sign_cusp[1] = cdata[ui->cc]->sign_cusp[1];
	
	wheel_init(ui->main_win, ui, 0, 9, 0);
	planet_pos(ui->main_win, cdata[key], ui, planet, zodiac);
	
	ui->bcc = ui->cc;
	ui->cc = TRANSIT;
	cc_data(ui->main_win, cdata[key], ui);
	ui->cc = ui->bcc;
	
	cdata[key]->sign_cusp[1] = tmp;
	
	wnoutrefresh(ui->main_win);
	doupdate();
	wheel_init(ui->main_win, ui, 0, 0, 0);
}
		
static void calc_return(struct cdata *cdata, double base_degree)
{
	struct tm temp = {0};
	time_t t = 0;

	cpt(cdata, &temp, &t, 0);
	
	int iflag = SEFLG_SWIEPH;
	double xx[6];
	char serr[AS_MAXCH];
	double diff;
	
	for(;;)
	{
		calculate_utc(cdata);
	
		double jd_ut = swe_julday(cdata->utc_year, cdata->utc_mon, 
		cdata->utc_mday, cdata->utc_hour, SE_GREG_CAL);
	
		swe_calc_ut(jd_ut, SE_SUN, iflag, xx, serr);
			
		diff = base_degree - xx[LONG];
		
		time_t step = 
			fabs(diff) > 2.0 ? 86400 :
			fabs(diff) > 0.1 ? 3600 :
			fabs(diff) > 0.002 ? 60 : 1;
			
		t+= (diff > 0.0 ? step : -step);
		
		cpt(cdata, &temp, &t, 1);
		
		if (fabs(diff) < 0.00001157407407)
			break;
	}
}
	
void solar_return(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac)
{
	double base_degree = pxx->dsun[LONG];
	
	int height = 5;
	int width = 30;
	
	int starty = (LINES- height) / 2;
	int startx = (COLS - width) / 2;
	
	WINDOW *sr_win = newwin(height, width, starty, startx);
	WINDOW *sr_subwin = derwin(sr_win, height - 2, width - 2, 0, 0);
	
	wbkgdset(sr_win, COLOR_PAIR(M_COLOR));
	
	cbreak();
	keypad(sr_win, TRUE);	
	
	FIELD *sr_field[2];
	sr_field[0] = new_field(1, 25, 2, 2, 0, 0);
	set_field_back(sr_field[0], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(sr_field[0], O_STATIC);
	field_opts_off(sr_field[0], O_AUTOSKIP);
	set_field_type(sr_field[0], TYPE_INTEGER, 0, -12998, 16799);
	
	sr_field[1] = NULL;
	
	FORM *sr_form = new_form(sr_field);
	set_form_win(sr_form, sr_win);
	set_form_sub(sr_form, sr_subwin);
	
	post_form(sr_form);
	box(sr_win, 0, 0);
	mvwaddstr(sr_win, 1, 1, "-o--year?-o");
	
	set_current_field(sr_form, sr_field[0]);
	wrefresh(sr_win);
	pos_form_cursor(sr_form);
	
	int done = 0, ch = 0, c = 0;
	while(!done && (ch = wgetch(sr_win)))
	{
		switch (ch)
		{
			case '\n':
				form_driver(sr_form, REQ_VALIDATION);
				done = 1;
				break;
			case KEY_BACKSPACE:
				form_driver(sr_form, REQ_DEL_PREV);
				break;
			case KEY_LEFT:
				form_driver(sr_form, REQ_LEFT_CHAR);
				break;
			case KEY_RIGHT:
				form_driver(sr_form, REQ_RIGHT_CHAR);
				break;
			case 27:
				c = 1;
				done = 1;
				break;
			default:
				form_driver(sr_form, ch);
				break;
		}
		wrefresh(sr_win);
	}
	
	if (!c)
	{
		char *endptr = NULL;
		long iret;
		errno = 0;
		
		char *sr_year = field_buffer(sr_field[0], 0);
		int len = 0;
		
		field_info(sr_field[0], NULL, &len, NULL, NULL, NULL, NULL);
		
		while (len > 0 && sr_year[len - 1] == ' ')
			len--;
		sr_year[len] = '\0';
		
		iret = strtol(sr_year, &endptr, 10);
		if (errno != ERANGE)
			cdata->year = (int)iret;
		else
			cdata->year = 1970;
	}

	werase(sr_subwin);
	wrefresh(sr_subwin);
	werase(sr_win);
	wrefresh(sr_win);
	delwin(sr_subwin);
	delwin(sr_win);
				
	unpost_form(sr_form);
	set_form_fields(sr_form, NULL);
	for (int i = 0; i < 2; ++i)
		free_field(sr_field[i]);
	free_form(sr_form);
	
	calc_return(cdata, base_degree);
	
	new_chart(cdata, pxx, ui, planet, zodiac);
	doupdate();
}

static void arrange_panel(struct cdata *cdata, struct pxx *pxx, struct ui *ui,
double **planet, int **zodiac)
{
	overwrite(ui->main_win, ui->transit_window);
	pxx_init(cdata, pxx, planet);
	wheel_init(ui->main_win, ui, 0, 9, 0);
	planet_pos(ui->transit_window, cdata, ui, planet, zodiac);
	cc_data(ui->transit_window, cdata, ui);
					
	top_panel(ui->main_panel);
	top_panel(ui->transit_panel);
	
	if (ui->left_trig)
		top_panel(ui->left_panel);
	if (ui->right_trig)
		top_panel(ui->right_panel);
		
	update_panels();
	doupdate();
}

void animate_chart(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac)
{
	int starty = 0;
	int startx = COLS - 14;
	
	mvwprintw(ui->main_win, starty, startx, "(hour)");
	if (ui->cc == TRANSIT)
		arrange_panel(cdata, pxx, ui, planet, zodiac);
	
	int max_day = 0; // daycount() return flag
	size_t inc = HOUR;
	
	struct tm temp = {0};
	time_t t = 0;
	
	cpt(cdata, &temp, &t, 0);
	
	int ch = 0;
	int anim_done = 0;
	while(!anim_done && (ch = wgetch(ui->main_win)))
	{
		if (ui->cc == TRANSIT)
			top_panel(ui->transit_panel);
			
		table_trigger(ui, ch);
	
		switch(ch)
		{
			case 'k': case KEY_UP:
				switch(inc)
				{
					case SECOND:
						t += 1;
						break;
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
						max_day = daycount(temp.tm_mon, temp.tm_year);
						if (temp.tm_mday > max_day)
							temp.tm_mday = max_day;
						t = mktime(&temp);
						ecst_init(planet, cdata->se);
						break;
					case YEAR:
						temp.tm_year++;
						if (temp.tm_year > 16799)
							temp.tm_year = -12998;
						t = mktime(&temp);
						ecst_init(planet, cdata->se);
						break;
				}
				cpt(cdata, &temp, &t, 1);
				
				if (ui->cc == TRANSIT)
					arrange_panel(cdata, pxx, ui, planet, zodiac);
				else
					new_chart(cdata, pxx, ui, planet, zodiac);
				break;
				
			case 'j': case KEY_DOWN:
				switch(inc)
				{
					case SECOND:
						t -= 1;
						break;
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
						max_day = daycount(temp.tm_mon, temp.tm_year);
						if (temp.tm_mday > max_day)
							temp.tm_mday = max_day;
						t = mktime(&temp);
						ecst_init(planet, cdata->se);
						break;
					case YEAR:
						--temp.tm_year;
						if (temp.tm_year < -12998)
							temp.tm_year = 16799;
						t = mktime(&temp);
						ecst_init(planet, cdata->se);
						break;
				}
				cpt(cdata, &temp, &t, 1);
				
				if (ui->cc == TRANSIT)
					arrange_panel(cdata, pxx, ui, planet, zodiac);
				else
					new_chart(cdata, pxx, ui, planet, zodiac);
				break;
				
			case 'h': case KEY_LEFT:
				if (inc != SECOND)
					++inc;
				break;
				
			case 'l': case KEY_RIGHT:
				if (inc != YEAR)
					--inc;
				break;
				
			case '\n': case 'q': case 't':
				anim_done = 1;
				break;
		}
		if (ui->cc == TRANSIT)
			arrange_panel(cdata, pxx, ui, planet, zodiac);
	
		flushinp();
		enanosleep(10);
		switch(inc)
		{
			case SECOND:
				wmove(ui->main_win, starty, startx);
				wclrtoeol(ui->main_win);
				mvwprintw(ui->main_win, starty, startx, "(sec)");
				break;
				
			case MINUTE:
				wmove(ui->main_win, starty, startx);
				wclrtoeol(ui->main_win);
				mvwprintw(ui->main_win, starty, startx, "(min)");
				break;
				
			case HOUR:
				wmove(ui->main_win, starty, startx);
				wclrtoeol(ui->main_win);
				mvwprintw(ui->main_win, starty, startx, "(hour)");
				break;
				
			case DAY:
				wmove(ui->main_win, starty, startx);
				wclrtoeol(ui->main_win);
				mvwprintw(ui->main_win, starty, startx, "(day)");
				break;
				
			case MONTH:
				wmove(ui->main_win, starty, startx);
				wclrtoeol(ui->main_win);
				mvwprintw(ui->main_win, starty, startx, "(mon)");
				break;
				
			case YEAR:
				wmove(ui->main_win, starty, startx);
				wclrtoeol(ui->main_win);
				mvwprintw(ui->main_win, starty, startx, "(year)");
				break;
		}
		if (ui->cc == TRANSIT)
		{
			copywin(ui->main_win, ui->transit_window, 
			starty, startx,
			starty, startx,
			starty, startx + 5,
			FALSE);
			update_panels();
			doupdate();
		}
	}
	wmove(ui->main_win, starty, startx);
	wclrtoeol(ui->main_win);
	wnoutrefresh(ui->main_win);
}
