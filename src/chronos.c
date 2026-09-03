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
#include <unistd.h>
#include <ncurses.h>
#include "swephexp.h"
#include "astro.h"
#include "chronos.h"

void set_localtime(struct cdata *cdata)
{	
	time_t now = time(NULL);
	struct tm gt = {0};
		
	localtime_r(&now, &gt);
	
	cdata->tm_year = gt.tm_year+1900;
	cdata->tm_mon = gt.tm_mon + 1;
	cdata->tm_mday = gt.tm_mday;
	cdata->tm_hour = gt.tm_hour;
	cdata->tm_min = gt.tm_min;
	cdata->tm_sec = gt.tm_sec;
	cdata->tm_wday = gt.tm_wday;
}

void weekday_check(struct cdata *cdata)
{
	struct tm gt = {0};
	
	gt.tm_year = cdata->tm_year - 1900;
	gt.tm_mon = cdata->tm_mon - 1;
	gt.tm_mday = cdata->tm_mday;
	gt.tm_hour = cdata->tm_hour - 1;
	gt.tm_min = cdata->tm_min;
	gt.tm_sec = cdata->tm_sec;
	
	mktime(&gt);
	cdata->tm_wday = gt.tm_wday;
}

int months(int month, int year)
{
	int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	if (month == 2)
		if (((year + 1900) % 4 == 0 && (year + 1900) % 100 != 0) || 
		((year + 1900) % 400 == 0))
			return 29;
	return days[month];
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
		pxx->dspir[LONG] = pxx->dasc[LONG] - diff;
	}
	else // night
	{
		pxx->dfor[LONG] = pxx->dasc[LONG] - diff;
		pxx->dspir[LONG] = pxx->dasc[LONG] + diff;
	}
	
	while (pxx->dfor[LONG] < 0.0)
		pxx->dfor[LONG] += 360.0;
	while (pxx->dfor[LONG] > 360.0)
		pxx->dfor[LONG] -= 360.0;
		
	while (pxx->dspir[LONG] < 0.0)
		pxx->dspir[LONG] += 360.0;
	while (pxx->dspir[LONG] > 360.0)
		pxx->dspir[LONG] -= 360.0;
}

void calculate_utc(struct cdata *cdata)
{
	struct tm tm_in = {0};
	tm_in.tm_year = cdata->tm_year - 1900;
	tm_in.tm_mon = cdata->tm_mon - 1;
	tm_in.tm_mday = cdata->tm_mday;
	tm_in.tm_hour = cdata->tm_hour;
	tm_in.tm_min = cdata->tm_min;
	tm_in.tm_sec = cdata->tm_sec;
	if (cdata->tm_isdst == 3)
		tm_in.tm_isdst = 1;
	else if (cdata->tm_isdst == 2)
		tm_in.tm_isdst = 0;
	else
		tm_in.tm_isdst = -1;
	
	time_t t = mktime(&tm_in);
	struct tm *result = localtime(&t);
	
	if (cdata->tm_isdst == 3)
		cdata->tm_isdst = 1;
	else if (cdata->tm_isdst == 2)
		cdata->tm_isdst = 0;
	else
		cdata->tm_isdst = result->tm_isdst;
	
	struct tm *tm_utc = gmtime(&t);
	
	cdata->utc_hour = tm_utc->tm_hour + tm_utc->tm_min / 60.0 +
	tm_utc->tm_sec / 3600.0;
	
	cdata->utc_year = tm_utc->tm_year + 1900;
	cdata->utc_mon = tm_utc->tm_mon + 1;
	cdata->utc_mday = tm_utc->tm_mday;
}

void retro_calc(double jd_ut, int ipl, double *planet[])
{
	int iflag = SEFLG_SWIEPH | SEFLG_SPEED;
	double xx[6];
	char serr[AS_MAXCH];
	
	const double parsemax = 4;
	const double parsemin = 0.5;
	
	double jd_copy = jd_ut;
	
	double speed = planet[ipl][LONG_S];
	
	int ns_found = 0;
	while(speed > 0.0 && !ns_found)
	{
		jd_copy += parsemax;
		swe_calc_ut(jd_copy, ipl, iflag, xx, serr);
		speed = xx[LONG_S];
		while (speed < 0.0)
		{
			jd_copy -= parsemin;
			swe_calc_ut(jd_copy, ipl, iflag, xx, serr);
			speed = xx[LONG_S];
			planet[ipl][NEXT_S] = jd_copy - jd_ut;
			planet[ipl][NEXT_JUL] = jd_copy;
			ns_found = 1;
		}
	}
	
	while(speed < 0.0 && !ns_found)
	{
		jd_copy += parsemax;
		swe_calc_ut(jd_copy, ipl, iflag, xx, serr);
		speed = xx[LONG_S];
		while (speed > 0.0)
		{
			jd_copy -= parsemin;
			swe_calc_ut(jd_copy, ipl, iflag, xx, serr);
			speed = xx[LONG_S];
			planet[ipl][NEXT_S] = jd_copy - jd_ut;
			planet[ipl][NEXT_JUL] = jd_copy;
			ns_found = 1;
		}
	}
	
	speed = planet[ipl][LONG_S];
	jd_copy = jd_ut;
	int ps_found = 0;
	while(speed > 0.0 && !ps_found)
	{
		jd_copy -= parsemax;
		swe_calc_ut(jd_copy, ipl, iflag, xx, serr);
		speed = xx[LONG_S];
		while (speed < 0.0)
		{
			jd_copy += parsemin;
			swe_calc_ut(jd_copy, ipl, iflag, xx, serr);
			speed = xx[LONG_S];
			planet[ipl][PREV_S] = jd_copy - jd_ut;
			planet[ipl][PREV_JUL] = jd_copy;
			ps_found = 1;
		}
	}
	
	while(speed < 0.0 && !ps_found)
	{
		jd_copy -= parsemax;
		swe_calc_ut(jd_copy, ipl, iflag, xx, serr);
		speed = xx[LONG_S];
		while (speed > 0.0)
		{
			jd_copy += parsemin;
			swe_calc_ut(jd_copy, ipl, iflag, xx, serr);
			speed = xx[LONG_S];
			planet[ipl][PREV_S] = jd_copy - jd_ut;
			planet[ipl][PREV_JUL] = jd_copy;
			ps_found = 1;
		}
	}
}

void retro_station(double jd_ut, double *planet[])
{
	const int station = 7;
	const double is_retro = 0.0;
	
	const int iter = 32;
	const int multi = 16;
	
	double limit[32] = {0};
	
	for (int ipl = SE_MERCURY; ipl <= SE_PLUTO; ipl++)
	{
		for (int i = 0; i < iter; ++i)
		{
			limit[i] = (multi * i);
			
			if (planet[ipl][NEXT_JUL] - jd_ut > 0.05 ||
			planet[ipl][PREV_JUL] - jd_ut < -0.05)
			{
				double nr = planet[ipl][NEXT_JUL] - jd_ut;
				planet[ipl][NEXT_S] = nr;
				double pr = planet[ipl][PREV_JUL] - jd_ut;
				planet[ipl][PREV_S] = pr;
			}
			if (fabs(planet[ipl][NEXT_S] - limit[i]) <= 0.0006 || 
			fabs(planet[ipl][PREV_S] - limit[i]) <= 0.0006)
			{
				planet[ipl][RET_INIT] = 0;
				break;
			}
			if (planet[ipl][NEXT_S] <= 0 || planet[ipl][PREV_S] >= 0)
			{
				planet[ipl][RET_INIT] = 0;
				break;
			}
		}
		
		if ((int)planet[ipl][RET_INIT] == 0)
		{
			retro_calc(jd_ut, ipl, planet);
			planet[ipl][RET_INIT] = 1;
		}
	
		// fill retro & station data
		if (planet[ipl][LONG_S] <= is_retro)
			planet[ipl][RETRO] = 1.0;
		else
			planet[ipl][RETRO] = 0.0;
			
		if ((int)planet[ipl][RETRO] == 1 && planet[ipl][NEXT_S] <= station)
			planet[ipl][STATION] = STATION_D;
		else if ((int)planet[ipl][RETRO] == 0 && planet[ipl][NEXT_S] <= station)
			planet[ipl][STATION] = STATION_R;
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
	
	const double eclipse_calc = 1.0;
	const int iter = 32;
	const int multi = 8;
	
	double limit[32] = {0};
	
	for (int i = 0; i < iter; ++i)
	{
		limit[i] = (multi * i);
		
		if (sol_eclipse[E_INIT] > 0)
		{
			double ens_jul = fabs(sol_eclipse[EN_FJUL] - jd_ut);
			sol_eclipse[EN_JUL] = ens_jul;
			double eps_jul = fabs(sol_eclipse[EP_FJUL] - jd_ut);
			sol_eclipse[EP_JUL] = eps_jul;
			
			double enl_jul = fabs(luna_eclipse[EN_FJUL] - jd_ut);
			luna_eclipse[EN_JUL] = enl_jul;
			double epl_jul = fabs(luna_eclipse[EP_FJUL] - jd_ut);
			luna_eclipse[EP_JUL] = epl_jul;
		}
		
		if (sol_eclipse[EN_JUL] < eclipse_calc || luna_eclipse[EN_JUL] < eclipse_calc ||
		sol_eclipse[EP_JUL] < eclipse_calc || luna_eclipse[EP_JUL] < eclipse_calc)
			sol_eclipse[E_INIT] = 0;
			
		if (fabs(limit[i] - sol_eclipse[EN_JUL]) <= eclipse_calc || fabs(limit[i] - sol_eclipse[EP_JUL]) <= eclipse_calc ||
		fabs(limit[i] - luna_eclipse[EN_JUL]) <= eclipse_calc || fabs(limit[i] - luna_eclipse[EP_JUL]) <= eclipse_calc ||
		(int)sol_eclipse[E_INIT] == 0)
		{
			swe_sol_eclipse_when_glob(jd_ut, iflag, 0, tret, NEXT_E, serr);
			swe_calc_ut(tret[0], SE_SUN, iflag, xx, serr);
			sol_eclipse[EN_JUL] = tret[0] - jd_ut;
			sol_eclipse[EN_FJUL] = tret[0];
			sol_eclipse[EN_SIGN] = ((int)xx[LONG] / 30) + 1;
			
			swe_sol_eclipse_when_glob(jd_ut, iflag, 0, tret, PREV_E, serr);
			swe_calc_ut(tret[0], SE_SUN, iflag, xx, serr);
			sol_eclipse[EP_JUL] = jd_ut - tret[0];
			sol_eclipse[EP_FJUL] = tret[0];
			sol_eclipse[EP_SIGN] = ((int)xx[LONG] / 30) + 1;
			
			swe_lun_eclipse_when(jd_ut, iflag, 0, tret, NEXT_E, serr);
			swe_calc_ut(tret[0], SE_MOON, iflag, xx, serr);
			luna_eclipse[EN_JUL] = tret[0] - jd_ut;
			luna_eclipse[EN_FJUL] = tret[0];
			luna_eclipse[EN_SIGN] = ((int)xx[LONG] / 30) + 1;
			
			swe_lun_eclipse_when(jd_ut, iflag, 0, tret, PREV_E, serr);
			swe_calc_ut(tret[0], SE_MOON, iflag, xx, serr);
			luna_eclipse[EP_JUL] = jd_ut - tret[0];
			luna_eclipse[EP_FJUL] = tret[0];
			luna_eclipse[EP_SIGN] = ((int)xx[LONG] / 30) + 1;
			
			sol_eclipse[E_INIT] = 1;
			break;
		}
	}
}
