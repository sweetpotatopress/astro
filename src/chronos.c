/* Copyright (C) 2026 yam lynn
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License
as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTIBILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. if not, see <https://www.gnu.org/licenses/> */

#include <math.h>
#include <time.h>
#include <unistd.h>
#include <swephexp.h>
#include <ncurses.h>
#include "astro.h"
#include "chronos.h"

void set_localtime(struct cdata *cdata)
{	
	time_t now = time(NULL);
	struct tm gettime = {0};
		
	localtime_r(&now, &gettime);
	
	cdata->tm_year = gettime.tm_year+1900;
	cdata->tm_mon = gettime.tm_mon + 1;
	cdata->tm_mday = gettime.tm_mday;
	cdata->tm_hour = gettime.tm_hour;
	cdata->tm_min = gettime.tm_min;
	cdata->tm_sec = gettime.tm_sec;
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
	int sect;
	double dist = pxx->dsun[LONG] - pxx->dasc[LONG];
	
	while (dist < 0)
		dist += 360;
	while (dist >= 360)
		dist -= 360;
		
	if (dist > 180)
		sect = DAY_SECT;
	else
		sect = NIGHT_SECT;
	return sect;
}

void lots(int sect, struct pxx *pxx)
{
	double offset = (360 - pxx->dsun[LONG]);
	double diff = (offset + pxx->dmoon[LONG]);
	while (diff > 360.0)
		diff -= 360.0;
	
	if (sect == DAY_SECT)
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
	tm_in.tm_isdst = -1;
	
	time_t t = mktime(&tm_in);
	struct tm *result = localtime(&t);
	
	cdata->tm_isdst = result->tm_isdst;
	
	struct tm *tm_utc = gmtime(&t);
	
	cdata->utc_hour = tm_utc->tm_hour + tm_utc->tm_min / 60.0 +
	tm_utc->tm_sec / 3600.0;
	
	cdata->utc_year = tm_utc->tm_year + 1900;
	cdata->utc_mon = tm_utc->tm_mon + 1;
	cdata->utc_mday = tm_utc->tm_mday;
}

void retro_calc(double jd_ut, int iter[],
int ipl, double *p_arr[])
{
	int iflag = SEFLG_SWIEPH | SEFLG_SPEED;
	double xx[6];
	char serr[AS_MAXCH];
	
	const double parsemax = 5;
	const double parsemin = 0.5;
	
	double jd_copy = jd_ut;
	
	double speed = p_arr[ipl][LONG_S];
	
	int retro_found = 0;
	while(speed > 0.0 && !retro_found)
	{
		jd_copy += parsemax;
		swe_calc_ut(jd_copy, ipl, iflag, xx, serr);
		speed = xx[LONG_S];
		while (speed < 0.0)
		{
			jd_copy -= parsemin;
			swe_calc_ut(jd_copy, ipl, iflag, xx, serr);
			speed = xx[LONG_S];
			p_arr[ipl][NEXT_R] = jd_copy - jd_ut;
			retro_found = 1;
		}
	}
	
	while(speed >= 0.0 && retro_found)
	{
		jd_copy += parsemax;
		swe_calc_ut(jd_copy, ipl, iflag, xx, serr);
		speed = xx[LONG_S];
	}
	
	int station_found = 0;
	while(speed < 0.0 && !station_found)
	{
		jd_copy += parsemax;
		swe_calc_ut(jd_copy, ipl, iflag, xx, serr);
		speed = xx[LONG_S];
		while (speed > 0.0)
		{
			jd_copy -= parsemin;
			swe_calc_ut(jd_copy, ipl, iflag, xx, serr);
			speed = xx[LONG_S];
			p_arr[ipl][NEXT_S] = jd_copy - jd_ut;
			station_found = 1;
		}
	
	}
	iter[ipl] = 0;
}

void retro_station(double jd_ut, double *p_arr[],
int *calc_flag, int *iter, double *last_jd)
{
	int ipl;
	const int itermax = 10;
	const int station = 7;
	const double is_retro = 0.5;
	
	for (ipl = SE_MERCURY; ipl <= SE_PLUTO; ipl++)
	{
		if ((p_arr[ipl][NEXT_R] < 50.0 && iter[ipl] >= itermax) ||
		(p_arr[ipl][NEXT_R] > 90.0 && iter[ipl] >= itermax))
			retro_calc(jd_ut, iter, ipl, p_arr);
			
		if (p_arr[ipl][NEXT_R] > -0.0001 && p_arr[ipl][NEXT_R] < 0.0001)
			retro_calc(jd_ut, iter, ipl, p_arr);
	
		if (calc_flag[ipl] == 0)
		{
			retro_calc(jd_ut, iter, ipl, p_arr);
			calc_flag[ipl] = 1;
			
			if (p_arr[ipl][LONG_S] < 0.0)
				p_arr[ipl][NEXT_R] = is_retro;
		}
		else if (fabs(*last_jd - jd_ut) >= 1.0)
		{
			double offset = fabs(*last_jd - jd_ut);
			
			if (*last_jd < jd_ut)
			{
				p_arr[ipl][NEXT_S] += offset;
				p_arr[ipl][NEXT_R] -= offset;
				if (p_arr[ipl][NEXT_R] <= 0.0)
					p_arr[ipl][NEXT_R] = is_retro;
			}
			else if (*last_jd > jd_ut)
			{
				p_arr[ipl][NEXT_S] += offset;
				if (p_arr[ipl][NEXT_R] > is_retro)
					p_arr[ipl][NEXT_R] += offset;
			}

			if (p_arr[ipl][LONG_S] < 0.0)
				p_arr[ipl][NEXT_R] = is_retro;
		}
		
		iter[ipl]++;
		
		// fill retro & station data
		if (p_arr[ipl][NEXT_R] <= is_retro)
			p_arr[ipl][RETRO] = 1;
		else
			p_arr[ipl][RETRO] = 0;
		if (p_arr[ipl][NEXT_S] <= station)
			p_arr[ipl][STATION] = STATION_D;
		else if (p_arr[ipl][NEXT_R] <=
		station && p_arr[ipl][NEXT_R] > is_retro)
			p_arr[ipl][STATION] = STATION_R;
		else
			p_arr[ipl][STATION] = 0;
	}
	*last_jd = jd_ut;
}

void eclipse(double jd_ut,
double *luna_eclipse, double *sol_eclipse)
{
	int iflag = SEFLG_SWIEPH;
	double tret[10];
	double xx[6];
	char serr[AS_MAXCH];
	
	const int iter = 128;
	const int eclipse_calc = 1;
	const int multi = 64;
	
	double limit[128] = {0};
	
	for (int i = 0; i < iter; ++i)
		limit[i] = (multi * i);
	
	for (int i = 0; i < iter; ++i)
	{
		int c = 0;
		if (c == 0 && sol_eclipse[E_INIT] > 0)
		{
			double ens_jul = fabs(sol_eclipse[EN_FJUL] - jd_ut);
			sol_eclipse[EN_JUL] = ens_jul;
			double eps_jul = fabs(sol_eclipse[EP_FJUL] - jd_ut);
			sol_eclipse[EP_JUL] = eps_jul;
			
			double enl_jul = fabs(luna_eclipse[EN_FJUL] - jd_ut);
			luna_eclipse[EN_JUL] = enl_jul;
			double epl_jul = fabs(luna_eclipse[EP_FJUL] - jd_ut);
			luna_eclipse[EP_JUL] = epl_jul;
			c++;
		}
		
		if (sol_eclipse[EN_JUL] < eclipse_calc || luna_eclipse[EN_JUL] < eclipse_calc ||
		sol_eclipse[EP_JUL] < eclipse_calc || luna_eclipse[EP_JUL] < eclipse_calc)
			ECLIPSE_INIT();
			
		if (fabs(limit[i] - sol_eclipse[EN_JUL]) < eclipse_calc ||
		(int)sol_eclipse[E_INIT] == 0)
		{
			swe_sol_eclipse_when_glob(jd_ut, iflag, 0, tret, NEXT_E, serr);
			swe_calc_ut(tret[0], SE_SUN, iflag, xx, serr);
			sol_eclipse[EN_JUL] = fabs(tret[0] - jd_ut);
			sol_eclipse[EN_FJUL] = tret[0];
			sol_eclipse[EN_SIGN] = ((int)xx[LONG] / 30) + 1;
			
			swe_sol_eclipse_when_glob(jd_ut, iflag, 0, tret, PREV_E, serr);
			swe_calc_ut(tret[0], SE_SUN, iflag, xx, serr);
			sol_eclipse[EP_JUL] = fabs(tret[0] - jd_ut);
			sol_eclipse[EP_FJUL] = tret[0];
			sol_eclipse[EP_SIGN] = ((int)xx[LONG] / 30) + 1;
			
			swe_lun_eclipse_when(jd_ut, iflag, 0, tret, NEXT_E, serr);
			swe_calc_ut(tret[0], SE_MOON, iflag, xx, serr);
			luna_eclipse[EN_JUL] = fabs(tret[0] - jd_ut);
			luna_eclipse[EN_FJUL] = tret[0];
			luna_eclipse[EN_SIGN] = ((int)xx[LONG] / 30) + 1;
			
			swe_lun_eclipse_when(jd_ut, iflag, 0, tret, PREV_E, serr);
			swe_calc_ut(tret[0], SE_MOON, iflag, xx, serr);
			luna_eclipse[EP_JUL] = fabs(tret[0] - jd_ut);
			luna_eclipse[EP_FJUL] = tret[0];
			luna_eclipse[EP_SIGN] = ((int)xx[LONG] / 30) + 1;
			
			sol_eclipse[E_INIT] = 1;
			break;
		}
	}
}
	
void pxx_init(double cusp[], double sign_cusp[],
double *luna_eclipse, double *sol_eclipse, double *p_arr[],
struct cdata *cdata, struct pxx *pxx)
{
	int iflag = SEFLG_SWIEPH | SEFLG_SPEED;
	int ipl, iret;
	double xx[6];
	char serr[AS_MAXCH];
	double ascmc[10];
	int ihsy = 'W';
	
	calculate_utc(cdata);
	
	double jd_ut = swe_julday(cdata->utc_year, cdata->utc_mon, 
	cdata->utc_mday, cdata->utc_hour, SE_GREG_CAL);
	
	for (ipl = SE_SUN; ipl <= SE_TRUE_NODE; ipl++)
	{
		iret = swe_calc_ut(jd_ut, ipl, iflag, xx, serr);
		if (iret < 0) 
			ERR_EXIT("ERR: swe_calc_ut failure");
			
		p_arr[ipl][LONG] = xx[LONG];
		p_arr[ipl][LAT] = xx[LAT];
		p_arr[ipl][DIST] = xx[DIST];
		p_arr[ipl][LONG_S] = xx[LONG_S];
		p_arr[ipl][LAT_S] = xx[LAT_S];
		p_arr[ipl][DIST_S] = xx[DIST_S];
	}
	
	int calc_flag[SE_PLUTO + 1] = {0};
	int iter[SE_PLUTO + 1] = {0};
	double last_jd = jd_ut;
	
	retro_station(jd_ut, p_arr, calc_flag, iter, &last_jd);
	
	eclipse(jd_ut, luna_eclipse, sol_eclipse);
	
	iret = swe_houses_ex(jd_ut, 0, cdata->dlat, cdata->dlon,
	'W', sign_cusp, ascmc);
	if (iret < 0)
		ERR_EXIT("ERR: swe_houses_ex failure");
		
	iret = swe_houses_ex(jd_ut, 0, cdata->dlat, cdata->dlon,
	ihsy, cusp, ascmc);
	if (iret < 0)
		ERR_EXIT("ERR: swe_houses_ex failure");
		
	// turn mean node into south node
	p_arr[SE_MEAN_NODE][LONG] = (p_arr[SE_TRUE_NODE][LONG] + 180);
	if (p_arr[SE_MEAN_NODE][LONG] >= 360)
		p_arr[SE_MEAN_NODE][LONG] -= 360;
	
	// calculates ic/mc and fills struct members
	double asc = ascmc[0];
	double dsc = (ascmc[0] + 180);
	if (dsc >= 360)
		dsc -= 360;
	double ic = (ascmc[1] + 180);
	if (ic >= 360)
		ic -= 360;
	double mc = ascmc[1];
	
	pxx->dasc[LONG] = asc;
	pxx->ddsc[LONG] = dsc;
	pxx->dic[LONG] = ic;
	pxx->dmc[LONG] = mc;
	
	int chart_sect = sect(pxx);
	lots(chart_sect, pxx);
	
	// seperate degree and minutes
	for (ipl = SE_SUN; ipl < SPXXMAX; ++ipl)
	{
		p_arr[ipl][DEGREE] = (int)p_arr[ipl][LONG] % 30;
		p_arr[ipl][MIN] = (int)((p_arr[ipl][LONG] - (int)p_arr[ipl][LONG]) * 60);
		
		p_arr[ipl][DEGREE_S] = (int)p_arr[ipl][LONG_S];
		p_arr[ipl][MIN_S] = (fabs(p_arr[ipl][LONG_S] - (int)p_arr[ipl][LONG_S]) * 60);
		if (p_arr[ipl][LONG_S] < 0)
			p_arr[ipl][DEGREE_S] *= -1;
	}
}
