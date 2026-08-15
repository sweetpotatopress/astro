/*This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License
as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTIBILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. if not, see <https://www.gnu.org/licenses/> */

#include <time.h>
#include "swephexp.h"
#include "astro.h"
#include "chronos.h"
#include "anim.h"
#include "draw.h"

#define SECOND 6
#define MINUTE 5
#define HOUR 4
#define DAY 3
#define MONTH 2
#define YEAR 1

void cur_chart_data(WINDOW *main_win, struct io *io, struct cdata *cdata)
{	
	int starty = (LINES / 2) - 4;
	int startx = (COLS / 2) - 4;
	
	if(io->filename)
		mvwprintw(main_win, starty, startx, "%s", io->filename);
	
	starty += 1;
	if (cdata->state && !isdigit((unsigned char)cdata->state[0]) &&
	strlen(cdata->state) > 1)
		mvwprintw(main_win, starty, startx, "%s, %s, %s", cdata->city, cdata->state, cdata->country);
		
	else if (cdata->country && cdata->city && strlen(cdata->country) > 0 && strlen(cdata->city) > 0)
		mvwprintw(main_win, starty, startx, "%s, %s", cdata->city, cdata->country);
		
	starty += 1;
	const char *month[] = 
	{ "err", "jan", "feb", "mar", "apr", "may", "jun",
	"jul", "aug", "sep", "oct", "nov", "dec" };
	
	const char *weekday[] = 
	{ "sun", "mon", "tue", "wed", "thu", "fri", "sat" };
	
	if(cdata->tm_year && cdata->tm_mon && cdata->tm_mday)
		mvwprintw(main_win, starty, startx, "%s.%02d.%02d, %s",
		month[cdata->tm_mon], cdata->tm_mday, cdata->tm_year, weekday[cdata->tm_wday]);
		
	starty += 1;
	if (cdata->tm_hour >= 0)
	{
		int hour = cdata->tm_hour;
		if (hour == 12)
			mvwprintw(main_win, starty, startx, "%02d:%02d:%02dPM", cdata->tm_hour, cdata->tm_min, cdata->tm_sec);
		else if (hour > 12)	
			mvwprintw(main_win, starty, startx, "%02d:%02d:%02dPM", cdata->tm_hour - 12, cdata->tm_min, cdata->tm_sec);
		else if (hour == 0)
			mvwprintw(main_win, starty, startx, "12:%02d:%02dAM", cdata->tm_min, cdata->tm_sec);
		else if (hour > 0 && hour < 12)
			mvwprintw(main_win, starty, startx, "%02d:%02d:%02dAM", cdata->tm_hour, cdata->tm_min, cdata->tm_sec);
	}
	starty += 1;
	int utc;
	
	if ((int)cdata->utc_hour == 0)
		utc = cdata->tm_hour - 24;
	else
		utc = cdata->tm_hour - (int)cdata->utc_hour;
	if (utc > 14)
		utc -= 24;
	if (utc < - 12)
		utc += 24;
		
	if (cdata->tm_isdst > 0)
		mvwprintw(main_win, starty, startx, "DST UTC%+02d", utc);
	else
		mvwprintw(main_win, starty, startx, "UTC%+02d", utc);
	
	starty += 1;
	if (fabs(cdata->dlat) > 1e-6)
		mvwprintw(main_win, starty, startx, "%f", cdata->dlat);
	
	starty += 1;
	if (fabs(cdata->dlon) > 1e-6)
		mvwprintw(main_win, starty, startx, "%f", cdata->dlon);
}

void new_chart(NEW_CHART_PARAM())
{
	pxx_init(cusp, sign_cusp, luna_eclipse, sol_eclipse, p_arr, cdata, pxx);
	draw_chart(main_win, cusp, sign_cusp, p_arr, z_arr, pxx, cdata,
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
		retro_table(retro_win, luna_eclipse, sol_eclipse,
		p_arr, z_arr, zo_sym, pl_sym);
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
			usleep(1000);
			if (ch == 9 || ch == 'q')
				break;
			if (ch == 'p' && *planet_trig == 0)
			{
				*planet_trig = 1;
				show_panel(*planet_panel);
				ch = 0;
				break;
			}
			else if (ch == 'p' && *planet_trig == 1)
			{
				*planet_trig = 0;
				hide_panel(*planet_panel);
				ch = 0;
				break;
			}
			if (ch == 'o' && *retro_trig == 0)
			{
				*retro_trig = 1;
				show_panel(*retro_panel);
				ch = 0;
				break;
			}
			else if (ch == 'o' && *retro_trig == 1)
			{
				*retro_trig = 0;
				hide_panel(*retro_panel);
				ch = 0;
				break;
			}
		}
		
		if (ch == 9 || ch == 'q')
			break;
	}
	wmove(main_win, 0, COLS - 14);
	wclrtoeol(main_win);
	nodelay(main_win, FALSE);
}

static void cpt(struct cdata *cdata, struct tm *temp, struct tm *result, time_t *t, bool x)
{
	if (!x)
	{
		temp->tm_year = cdata->tm_year - 1900;
		temp->tm_mon = cdata->tm_mon - 1;
		temp->tm_mday = cdata->tm_mday;
		temp->tm_hour = cdata->tm_hour;
		temp->tm_min = cdata->tm_min;
		temp->tm_sec = cdata->tm_sec;
		temp->tm_isdst = cdata->tm_isdst;
		temp->tm_wday = cdata->tm_wday;
		
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
		cdata->tm_isdst = result->tm_isdst;
		cdata->tm_wday = result->tm_wday;
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
		
		ECLIPSE_INIT();
		pxx_init(cusp, sign_cusp, luna_eclipse, sol_eclipse, p_arr, cdata, pxx);
		
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
			
			pxx_init(cusp, sign_cusp, luna_eclipse, sol_eclipse, p_arr, cdata, pxx);
			cur_chart_data(main_win, io, cdata);
		}
	}
	mvwprintw(main_win, 0, COLS - 14, "              ");
}

void animate_chart(NEW_CHART_PARAM())
{
	int starty = 0;
	int startx = COLS - 14;
	
	mvwprintw(main_win, starty, startx, "(hour)");
	
	int max_day = 0; // months() return flag
	size_t i = HOUR; // time inc/dec
	
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
				if (i != SECOND)
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
					retro_table(retro_win, luna_eclipse, sol_eclipse,
					p_arr, z_arr, zo_sym, pl_sym);
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
					retro_table(retro_win, luna_eclipse, sol_eclipse,
					p_arr, z_arr, zo_sym, pl_sym);
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
						max_day = months(temp.tm_mon, temp.tm_year);
						if (temp.tm_mday > max_day)
							temp.tm_mday = max_day;
						t = mktime(&temp);
						ECLIPSE_INIT();
						break;
					case YEAR:
						temp.tm_year++;
						if (temp.tm_year > 16799)
							temp.tm_year = -12998;
						t = mktime(&temp);
						ECLIPSE_INIT();
					break;
				}
				cpt(cdata, &temp, result, &t, 1);
				
				new_chart(NEW_CHART_ARG());
				break;
			case 'j': case KEY_DOWN:
				switch(i)
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
						max_day = months(temp.tm_mon, temp.tm_year);
						if (temp.tm_mday > max_day)
							temp.tm_mday = max_day;
						t = mktime(&temp);
						ECLIPSE_INIT();
						break;
					case YEAR:
						--temp.tm_year;
						if (temp.tm_year < -12998)
							temp.tm_year = 16799;
						t = mktime(&temp);
						ECLIPSE_INIT();
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
			case SECOND:
				wmove(main_win, starty, startx);
				wclrtoeol(main_win);
				mvwprintw(main_win, starty, startx, "(sec)");
				break;
				
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
