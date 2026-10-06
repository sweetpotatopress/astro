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
#include "ui.h"
#include "chronos.h"
#include "anim.h"
#include "draw.h"
#include "init.h"

#define SECOND 6
#define MINUTE 5
#define HOUR 4
#define DAY 3
#define MONTH 2
#define YEAR 1

#define MIN_YEAR -14098
#define MAX_YEAR 14098

static void enanosleep(unsigned int ms)
{
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	nanosleep(&ts, NULL);
}

void cc_data(WINDOW *win, struct cdata *cdata, struct ui *ui)
{	
	int sy, sx;
	int count = (ui->cc == TRANSIT || ui->cc == SYNASTRY) ? 2 : 1;
	for (int i = 0; i < count; ++i)
	{
		if (ui->cc == TRANSIT || ui->cc == SYNASTRY)
		{
			i++;
			sy = 2;
			sx = COLS - 20;
		}
		else if (ui->left_trig)
		{
			sy = 1;
			sx = 33;
		}
		else if (!ui->left_trig)
		{
			sy = 1;
			sx = 1;
		}
		
		if(cdata->chart_name)
			mvwprintw(win, sy, sx, "%s", cdata->chart_name);
		
		if (ui->cc != TRANSIT || ui->cc != SYNASTRY)
		{
			sy += 1;
			if (cdata->state && !isdigit((unsigned char)cdata->state[0]) && strlen(cdata->state) > 1)
				mvwprintw(win, sy, sx, "%.22s, %s, %s", cdata->city, cdata->state, cdata->country);
			
			else if (cdata->country && cdata->city && strlen(cdata->country) > 0 && strlen(cdata->city) > 0)
				mvwprintw(win, sy, sx, "%.22s, %s", cdata->city, cdata->country);
		}
			
		sy += 1;
		
		if(cdata->year && cdata->mon && cdata->mday)
			mvwprintw(win, sy, sx, "%s.%02d.%02d, %s", 
			ui->sym.month[cdata->mon], cdata->mday, cdata->year, ui->sym.week[cdata->wday]);
			
		sy += 1;
		if (cdata->hour >= 0)
		{
			int hour = cdata->hour;
			if (hour == 12)
				mvwprintw(win, sy, sx, "%02d:%02d:%02dPM", cdata->hour, cdata->min, cdata->sec);
			else if (hour > 12)	
				mvwprintw(win, sy, sx, "%02d:%02d:%02dPM", cdata->hour - 12, cdata->min, cdata->sec);
			else if (hour == 0)
				mvwprintw(win, sy, sx, "12:%02d:%02dAM", cdata->min, cdata->sec);
			else if (hour > 0 && hour < 12)
				mvwprintw(win, sy, sx, "%02d:%02d:%02dAM", cdata->hour, cdata->min, cdata->sec);
		}
		sy += 1;
		
		int local_min = cdata->hour * 60 + cdata->min + (int)lround(cdata->sec / 60.0);
		int utc_min = (int)lround(cdata->utc_hour * 60.0);
		int off_min = local_min - utc_min;
		if (off_min < -12 * 60)
			off_min += 24 + 60;
		else if (off_min > 14 * 60)
			off_min -= 24 * 60;
		int usign = off_min < 0 ? -1 : 1;
		int abs_min = abs(off_min);
		int off_hour = abs_min / 60;
		int rem_min = abs_min % 60;
		
		mvwprintw(win, sy, sx, "%sUTC%c%02d:%02d", cdata->isdst == YDST ? "DST " : "",
		usign < 0 ? '-' : '+', off_hour, rem_min);
			
		if (i > 0)
			return;
			
		sy += 1;
		if (cdata->timezone)
			mvwprintw(win, sy, sx, "%.30s", cdata->timezone);
		
		sy += 1;
		if (fabs(cdata->dlat) > 1e-6)
			mvwprintw(win, sy, sx, "%f", cdata->dlat);
		
		sy += 1;
		if (fabs(cdata->dlon) > 1e-6)
			mvwprintw(win, sy, sx, "%f", cdata->dlon);
	}
}

void realtime_chart(struct cdata **cdata, struct pxx **pxx, struct ui *ui, double **planet, int **zodiac)
{
	nodelay(ui->main_win, TRUE);
		
	int ch = 0;
	while ((ch = wgetch(ui->main_win)) != 9)
	{
		set_localtime(cdata[ui->cc]);
		wheel_init(ui->main_win, ui, 0, 0, 0);
		new_chart(cdata[ui->cc], pxx[ui->cc], ui, planet, zodiac);
		
		wattron(ui->main_win, COLOR_PAIR(FIRE));
		mvwprintw(ui->main_win, 0, COLS - 14, "*live");
		wattroff(ui->main_win, COLOR_PAIR(FIRE));
		
		wnoutrefresh(ui->main_win);
	
		table_trigger(ui, ch);
		for (int i = 0; i < 10; ++i)
		{
			enanosleep(10);
			if (ch == 9 || ch == 'q')
				break;
		}
		doupdate();
		
		if (ch == 9 || ch == 'q')
		{
			cdata[ui->cc]->rt = 0;
			break;
		}
		if (isdigit(ch))
		{
			ui->bcc = ui->cc;
			set_chart(cdata, pxx, ui, planet, zodiac, ch);
			cdata[ui->bcc]->rt = 1;
			break;
		}
	}
	wmove(ui->main_win, 0, COLS - 14);
	wclrtoeol(ui->main_win);
	nodelay(ui->main_win, FALSE);
}

void transit(struct cdata **cdata, struct pxx **pxx, struct ui *ui, double **planet, int **zodiac)
{
	ui->transit_win = newwin(LINES, COLS, 0, 0);
	ui->transit_panel = new_panel(ui->transit_win);
	
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
	delwin(ui->transit_win);
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
	
	ui->bcc = ui->cc;
	ui->cc = SYNASTRY;
	planet_pos(ui->main_win, cdata[key], ui, planet, zodiac);
	cc_data(ui->main_win, cdata[key], ui);
	ui->cc = ui->bcc;
	
	cdata[key]->sign_cusp[1] = tmp;
	
	wnoutrefresh(ui->main_win);
	doupdate();
	wheel_init(ui->main_win, ui, 0, 0, 0);
}

static int daycount(int month, int year)
{
	const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	if (month == 2)
		if (((year + 1900) % 4 == 0 && (year + 1900) % 100 != 0) || 
		((year + 1900) % 400 == 0))
			return 29;
	return days[month];
}

static void print_inc(WINDOW *win, int sy, int sx, const char *label, int advance)
{
	wmove(win, sy, sx);
	wclrtoeol(win);
	mvwprintw(win, sy, sx, "(%s)[%d]", label, advance);
}

static void arrange_panel(struct cdata *cdata, struct pxx *pxx, struct ui *ui,
double **planet, int **zodiac, int sy, int sx, const char *label, int advance)
{
	overwrite(ui->main_win, ui->transit_win);
	pxx_init(cdata, pxx, planet);
	wheel_init(ui->main_win, ui, 0, 9, 0);
	planet_pos(ui->transit_win, cdata, ui, planet, zodiac);
	cc_data(ui->transit_win, cdata, ui);
					
	top_panel(ui->main_panel);
	top_panel(ui->transit_panel);
	
	if (ui->left_trig)
		top_panel(ui->left_panel);
	if (ui->right_trig)
		top_panel(ui->right_panel);
	print_inc(ui->transit_win, sy, sx, label, advance);
		
	update_panels();
	doupdate();
}

static void year_wrap(int *year)
{
	if (*year > MAX_YEAR)
		*year = MIN_YEAR;
	else if (*year < MIN_YEAR)
		*year = MAX_YEAR;
}

static void lesser_wrap(struct tm *temp, time_t *t, size_t inc)
{
	if (inc == SECOND || inc == MINUTE || inc == HOUR || inc == DAY)
	{
		struct tm *result = localtime(t);
		if (!result)
			return;
		*temp = *result;
		year_wrap(&temp->tm_year);
		*t = mktime(temp);
	}
}

void animate_chart(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac)
{
	int sy = 0;
	int sx = COLS - 14;
	
	int advance = 1;
	int next_advance = 0;
	const char *label = "hour";
	
	print_inc(ui->main_win, sy, sx, label, advance);
	if (ui->cc == TRANSIT)
		arrange_panel(cdata, pxx, ui, planet, zodiac, sy, sx, label, advance);
	
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
			
		if (table_trigger(ui, ch) == 1)
		{
			new_chart(cdata, pxx, ui, planet, zodiac);
			doupdate();
		}
		
		if (isdigit((unsigned char)ch))
		{
			int digit = ch - '0';
			
			if (!next_advance)
			{
				advance = digit;
				next_advance = 1;
			}
			else if (advance <= (INT_MAX - digit) / 10)
				advance = advance * 10 + digit;
			if (advance < 0 || advance > 999)
			{
				advance = 1;
				next_advance = 0;
			}
		}
		else
			next_advance = 0;
	
		switch(ch)
		{
			case 'k': case KEY_UP:
				switch(inc)
				{
					case SECOND:
						t += advance;
						break;
					case MINUTE:
						t += 60 * advance;
						break;
					case HOUR:
						t += 3600 * advance;
						break;
					case DAY:
						t += 86400 * advance;
						break;
					case MONTH:
					{
						int tot = temp.tm_mon + advance;
						temp.tm_year += tot / 12;
						temp.tm_mon = tot % 12;
						
						max_day = daycount(temp.tm_mon, temp.tm_year);
						if (temp.tm_mday > max_day)
							temp.tm_mday = max_day;
						year_wrap(&temp.tm_year);
						t = mktime(&temp);
						ecst_init(planet, cdata->se);
						break;
					}
					case YEAR:
						temp.tm_year += advance;
						year_wrap(&temp.tm_year);
						t = mktime(&temp);
						ecst_init(planet, cdata->se);
						break;
				}
				lesser_wrap(&temp, &t, inc);
				cpt(cdata, &temp, &t, 1);
				
				if (ui->cc == TRANSIT)
					arrange_panel(cdata, pxx, ui, planet, zodiac, sy, sx, label, advance);
				else
					new_chart(cdata, pxx, ui, planet, zodiac);
				break;
				
			case 'j': case KEY_DOWN:
				switch(inc)
				{
					case SECOND:
						t -= advance;
						break;
					case MINUTE:
						t -= 60 * advance;
						break;
					case HOUR:
						t -= 3600 * advance;
						break;
					case DAY:
						t -= 86400 * advance;
						break;
					case MONTH:
					{
						int tot = temp.tm_mon - advance;
						temp.tm_year += tot / 12;
						temp.tm_mon = tot % 12;
						if (temp.tm_mon < 0)
						{
							temp.tm_mon += 12;
							--temp.tm_year;
						}
						max_day = daycount(temp.tm_mon, temp.tm_year);
						if (temp.tm_mday > max_day)
							temp.tm_mday = max_day;
						year_wrap(&temp.tm_year);
						t = mktime(&temp);
						ecst_init(planet, cdata->se);
					}
						break;
					case YEAR:
						temp.tm_year -= advance;
						year_wrap(&temp.tm_year);
						t = mktime(&temp);
						ecst_init(planet, cdata->se);
						break;
				}
				lesser_wrap(&temp, &t, inc);
				cpt(cdata, &temp, &t, 1);
				
				if (ui->cc == TRANSIT)
					arrange_panel(cdata, pxx, ui, planet, zodiac, sy, sx, label, advance);
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
			arrange_panel(cdata, pxx, ui, planet, zodiac, sy, sx, label, advance);
	
		flushinp();
		enanosleep(10);
		switch(inc)
		{
			case SECOND:
				label = "sec";
				break;
				
			case MINUTE:
				label = "min";
				break;
				
			case HOUR:
				label = "hour";
				break;
				
			case DAY:
				label = "day";
				break;
				
			case MONTH:
				label = "mon";
				break;
				
			case YEAR:
				label = "year";
				break;
		}
		print_inc(ui->main_win, sy, sx, label, advance);
		
		if (ui->cc == TRANSIT)
		{
			copywin(ui->main_win, ui->transit_win, 
			sy, sx,
			sy, sx,
			sy, sx + 5,
			FALSE);
			print_inc(ui->transit_win, sy, sx, label, advance);
			update_panels();
			doupdate();
		}
	}
	wmove(ui->main_win, sy, sx);
	wclrtoeol(ui->main_win);
	wnoutrefresh(ui->main_win);
}
