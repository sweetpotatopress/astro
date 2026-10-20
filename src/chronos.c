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
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <ncurses.h>
#include "swephexp.h"
#include "astro.h"
#include "ui.h"
#include "init.h"
#include "chronos.h"

void set_localtime(struct cdata *cdata)
{	
	time_t now = time(NULL);
	struct tm gt = {0};
		
	localtime_r(&now, &gt);
	
	cdata->year = gt.tm_year+1900;
	cdata->mon = gt.tm_mon + 1;
	cdata->mday = gt.tm_mday;
	cdata->hour = gt.tm_hour;
	cdata->min = gt.tm_min;
	cdata->sec = gt.tm_sec;
	cdata->wday = gt.tm_wday;
	cdata->isdst = gt.tm_isdst;
}

int time_format(int th)
{
	if (th == 0)
		th = 12;
	if (th > 12)
		th -= 12;
	return th;
}

void weekday_check(struct cdata *cdata)
{
	struct tm gt = {0};
	
	gt.tm_year = cdata->year - 1900;
	gt.tm_mon = cdata->mon - 1;
	gt.tm_mday = cdata->mday;
	gt.tm_hour = cdata->hour - 1;
	gt.tm_min = cdata->min;
	gt.tm_sec = cdata->sec;
	
	mktime(&gt);
	cdata->wday = gt.tm_wday;
}

int sect(struct pxx *pxx)
{
	int sect = 0;
	double dist = pxx->dsun[LONG] - pxx->dasc[LONG];
	
	while (dist < 0.0)
		dist += 360.0;
	while (dist >= 360.0)
		dist -= 360.0;
	
	return sect = (dist > 180.0) ? DAY_SECT : NIGHT_SECT;
}

void lots(struct pxx *pxx)
{
	int chart_sect = sect(pxx);
	double offset = (360 - pxx->dsun[LONG]);
	double diff = (offset + pxx->dmoon[LONG]);
	while (diff > 360.0)
		diff -= 360.0;
	
	if (chart_sect == DAY_SECT)
	{
		pxx->dfor[LONG] = pxx->dasc[LONG] + diff;
		pxx->ddai[LONG] = pxx->dasc[LONG] - diff;
	}
	else // night
	{
		pxx->dfor[LONG] = pxx->dasc[LONG] - diff;
		pxx->ddai[LONG] = pxx->dasc[LONG] + diff;
	}
	
	while (pxx->dfor[LONG] < 0.0)
		pxx->dfor[LONG] += 360.0;
	while (pxx->dfor[LONG] > 360.0)
		pxx->dfor[LONG] -= 360.0;
		
	while (pxx->ddai[LONG] < 0.0)
		pxx->ddai[LONG] += 360.0;
	while (pxx->ddai[LONG] > 360.0)
		pxx->ddai[LONG] -= 360.0;
}

static int lmt_check(const struct cdata *cdata, const struct lmt *rule)
{
    if (cdata->year != rule->year)
        return cdata->year < rule->year;

    if (cdata->mon != rule->mon)
        return cdata->mon < rule->mon;

    if (cdata->mday != rule->mday)
        return cdata->mday < rule->mday;

    if (cdata->hour != rule->hour)
        return cdata->hour < rule->hour;

    if (cdata->min != rule->min)
        return cdata->min < rule->min;

    return cdata->sec < rule->sec;
}

static int is_lmt_date(const struct cdata *cdata)
{
    for (size_t i = 0; i < sizeof(dates) / sizeof(dates[0]); ++i)
    {
        if (strcmp(cdata->timezone, dates[i].timezone) == 0)
			return lmt_check(cdata, &dates[i]);
    }
    return 0;
}

void calculate_utc(struct cdata *cdata)
{
	struct tm tm_in = {0};
	tm_in.tm_year = cdata->year - 1900;
	tm_in.tm_mon = cdata->mon - 1;
	tm_in.tm_mday = cdata->mday;
	tm_in.tm_hour = cdata->hour;
	tm_in.tm_min = cdata->min;
	tm_in.tm_sec = cdata->sec;
	
	time_t t = mktime(&tm_in);
	cdata->isdst = tm_in.tm_isdst;
	
	if (is_lmt_date(cdata))
	{
		int offset = (int)lround(cdata->dlon * 240.0);
		t += (int)tm_in.tm_gmtoff - offset;
	}
	
	struct tm *tm_utc = gmtime(&t);
	
	cdata->utc_hour = tm_utc->tm_hour + tm_utc->tm_min / 60.0 +
	tm_utc->tm_sec / 3600.0;
	
	cdata->utc_year = tm_utc->tm_year + 1900;
	cdata->utc_mon = tm_utc->tm_mon + 1;
	cdata->utc_mday = tm_utc->tm_mday;
}

void cpt(struct cdata *cdata, struct tm *temp, time_t *t, bool x)
{
	if (x == IN)
	{
		temp->tm_year = cdata->year - 1900;
		temp->tm_mon = cdata->mon - 1;
		temp->tm_mday = cdata->mday;
		temp->tm_hour = cdata->hour;
		temp->tm_min = cdata->min;
		temp->tm_sec = cdata->sec;
		temp->tm_isdst = cdata->isdst;
		
		*t = mktime(temp);
	}
	else if (x == OUT)
	{	
		struct tm *result = localtime(t);
		if(!result)
			return;
		cdata->year = result->tm_year + 1900;
		cdata->mon = result->tm_mon + 1;
		cdata->mday = result->tm_mday;
		cdata->hour = result->tm_hour;
		cdata->min = result->tm_min;
		cdata->sec = result->tm_sec;
		cdata->isdst = result->tm_isdst;
		cdata->wday = result->tm_wday;
	}
}

static int speed_sign(double speed)
{
	if (speed > 0.0)
		return 1;
	if (speed < 0.0)
		return -1;
	return 0;
}

static void find_station(double jd_start, double initial_speed, int direction, int ipl, double *offset, double *jd, double *longitude)
{
	const double coarse = 2.0;
	const double fine = 0.1;
	
	double jd_ut = jd_start;
	int iflag = SEFLG_SWIEPH | SEFLG_SPEED;
	double xx[6];
	char serr[AS_MAXCH];
	double speed = initial_speed;
	
	int initial_sign = speed_sign(initial_speed);
	if (initial_sign == 0)
		return;
		
	while (speed_sign(speed) == initial_sign)
	{
		jd_ut += direction * coarse;
		swe_calc_ut(jd_ut, ipl, iflag, xx, serr);
		speed = xx[LONG_S];
	} 
		
	while (speed_sign(speed) != initial_sign)
	{
		jd_ut -= direction * fine;
		swe_calc_ut(jd_ut, ipl, iflag, xx, serr);
		speed = xx[LONG_S];
	} 
	
	*offset = jd_ut - jd_start;
	*jd = jd_ut;
	*longitude = xx[LONG];
}

void retro_station(double jd_ut, double *planet[])
{
	const int station = 7;
	
	for (int ipl = SE_MERCURY; ipl <= SE_PLUTO; ipl++)
	{
		if (planet[ipl][NEXT_JUL] - jd_ut > JUL_SEC ||
		planet[ipl][PREV_JUL] - jd_ut < -JUL_SEC)
		{
			planet[ipl][NEXT_S] = planet[ipl][NEXT_JUL] - jd_ut;
			planet[ipl][PREV_S] = planet[ipl][PREV_JUL] - jd_ut;
		}
		
		if (planet[ipl][NEXT_S] <= 0.0 || planet[ipl][PREV_S] >= 0.0)
			planet[ipl][RET_INIT] = 0;
		
		if (planet[ipl][RET_INIT] < 0.5)
		{
			find_station(jd_ut, planet[ipl][LONG_S], +1, ipl, &planet[ipl][NEXT_S], &planet[ipl][NEXT_JUL], &planet[ipl][NEXT_Z]);
			find_station(jd_ut, planet[ipl][LONG_S], -1, ipl, &planet[ipl][PREV_S], &planet[ipl][PREV_JUL], &planet[ipl][PREV_Z]);
			planet[ipl][RET_INIT] = 1;
		}
	
		const int is_retro = planet[ipl][LONG_S] <= 0.0;
		
		planet[ipl][RETRO] = is_retro ? 1.0 : 0.0;
		
		if (planet[ipl][NEXT_S] <= station)
			planet[ipl][STATION] = is_retro ? STATION_D : STATION_R;
		else
			planet[ipl][STATION] = 0.0;
	}
}

void eclipse(double jd_ut, double *luna_eclipse, double *sol_eclipse)
{
	int iflag = SEFLG_SWIEPH;
	double tret[10];
	double xx[6];
	char serr[AS_MAXCH];
	
	if (sol_eclipse[E_INIT] > 0)
	{
		sol_eclipse[EN_JUL] = sol_eclipse[EN_FJUL] - jd_ut;
		sol_eclipse[EP_JUL] = jd_ut - sol_eclipse[EP_FJUL];
		
		luna_eclipse[EN_JUL] = luna_eclipse[EN_FJUL] - jd_ut;
		luna_eclipse[EP_JUL] = jd_ut - luna_eclipse[EP_FJUL];
	}
	
	if (sol_eclipse[EN_JUL] < JUL_SEC || luna_eclipse[EN_JUL] < JUL_SEC ||
	sol_eclipse[EP_JUL] < JUL_SEC || luna_eclipse[EP_JUL] < JUL_SEC)
		sol_eclipse[E_INIT] = 0.0;
		
	if (sol_eclipse[E_INIT] < 0.5)
	{
		swe_sol_eclipse_when_glob(jd_ut, iflag, 0, tret, NEXT_E, serr);
		swe_calc_ut(tret[0], SE_SUN, iflag, xx, serr);
		sol_eclipse[EN_JUL] = tret[0] - jd_ut;
		sol_eclipse[EN_FJUL] = tret[0];
		sol_eclipse[EN_SIGN] = ((int)xx[LONG] / 30);
		
		swe_sol_eclipse_when_glob(jd_ut, iflag, 0, tret, PREV_E, serr);
		swe_calc_ut(tret[0], SE_SUN, iflag, xx, serr);
		sol_eclipse[EP_JUL] = jd_ut - tret[0];
		sol_eclipse[EP_FJUL] = tret[0];
		sol_eclipse[EP_SIGN] = ((int)xx[LONG] / 30);
		
		swe_lun_eclipse_when(jd_ut, iflag, 0, tret, NEXT_E, serr);
		swe_calc_ut(tret[0], SE_MOON, iflag, xx, serr);
		luna_eclipse[EN_JUL] = tret[0] - jd_ut;
		luna_eclipse[EN_FJUL] = tret[0];
		luna_eclipse[EN_SIGN] = ((int)xx[LONG] / 30);
		
		swe_lun_eclipse_when(jd_ut, iflag, 0, tret, PREV_E, serr);
		swe_calc_ut(tret[0], SE_MOON, iflag, xx, serr);
		luna_eclipse[EP_JUL] = jd_ut - tret[0];
		luna_eclipse[EP_FJUL] = tret[0];
		luna_eclipse[EP_SIGN] = ((int)xx[LONG] / 30);
		
		sol_eclipse[E_INIT] = 1.0;
	}
}

static void calc_return(struct cdata *cdata, double base_degree)
{
	struct tm temp = {0};
	time_t t = 0;

	cpt(cdata, &temp, &t, IN);
	
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
		
		cpt(cdata, &temp, &t, OUT);
		
		if (fabs(diff) < JUL_SEC)
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
