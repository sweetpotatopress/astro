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

void retro_calc(double jd_ut, int iter[],
int ipl, double *p_arr[])
{
	int iflag;
	double xx[6];
	char serr[AS_MAXCH];
	
	iflag = SEFLG_SWIEPH | SEFLG_SPEED;
	
	double jd_copy = jd_ut;
	
	double speed = p_arr[ipl][LONG_S];
	int retro_found = 0;
	while(speed > 0.0 && !retro_found)
	{
		jd_copy += PARSEMAX;
		swe_calc_ut(jd_copy, ipl, iflag, xx, serr);
		speed = xx[LONG_S];
		while (speed < 0.0)
		{
			jd_copy -= PARSEMIN;
			swe_calc_ut(jd_copy, ipl, iflag, xx, serr);
			speed = xx[LONG_S];
			p_arr[ipl][NEXT_R] = jd_copy - jd_ut;
			retro_found = 1;
		}
	}
	
	while(speed >= 0.0 && retro_found)
	{
		jd_copy += PARSEMAX;
		swe_calc_ut(jd_copy, ipl, iflag, xx, serr);
		speed = xx[LONG_S];
	}
	
	int station_found = 0;
	while(speed < 0.0 && !station_found)
	{
		jd_copy += PARSEMAX;
		swe_calc_ut(jd_copy, ipl, iflag, xx, serr);
		speed = xx[LONG_S];
		while (speed > 0.0)
		{
			jd_copy -= PARSEMIN;
			swe_calc_ut(jd_copy, ipl, iflag, xx, serr);
			speed = xx[LONG_S];
			p_arr[ipl][NEXT_S] = jd_copy - jd_ut;
			station_found = 1;
		}
	
	}
	iter[ipl] = 0;
}

void next_retro_station(double jd_ut, double *p_arr[],
int *calc_flag, int *iter, double *last_jd)
{
	int ipl;

	for (ipl = SE_MERCURY; ipl <= SE_PLUTO; ipl++)
	{
		if ((p_arr[ipl][NEXT_R] < 50.0 && iter[ipl] >= ITERMAX) ||
		(p_arr[ipl][NEXT_R] > 90.0 && iter[ipl] >= ITERMAX))
			retro_calc(jd_ut, iter, ipl, p_arr);
			
		if (p_arr[ipl][NEXT_R] > -0.0001 && p_arr[ipl][NEXT_R] < 0.0001)
			retro_calc(jd_ut, iter, ipl, p_arr);
	
		if (calc_flag[ipl] == 0)
		{
			retro_calc(jd_ut, iter, ipl, p_arr);
			calc_flag[ipl] = 1;
			
			if (p_arr[ipl][LONG_S] < 0.0)
				p_arr[ipl][NEXT_R] = IS_RETRO;
		}
		else if (fabs(*last_jd - jd_ut) >= 1.0)
		{
			double offset = fabs(*last_jd - jd_ut);
			
			if (*last_jd < jd_ut)
			{
				p_arr[ipl][NEXT_S] += offset;
				p_arr[ipl][NEXT_R] -= offset;
				if (p_arr[ipl][NEXT_R] <= 0.0)
					p_arr[ipl][NEXT_R] = IS_RETRO;
			}
			else if (*last_jd > jd_ut)
			{
				p_arr[ipl][NEXT_S] += offset;
				if (p_arr[ipl][NEXT_R] > IS_RETRO)
					p_arr[ipl][NEXT_R] += offset;
			}

			if (p_arr[ipl][LONG_S] < 0.0)
				p_arr[ipl][NEXT_R] = IS_RETRO;
		}
		
		iter[ipl]++;
	}
	*last_jd = jd_ut;
}
	
int sect(struct pxx *pxx)
{
	int sect = NIGHT_SECT;
	
	if ((pxx->dsun[LONG] - pxx->dasc[LONG]) <= 180)
		sect = DAY_SECT;
	else
		sect = NIGHT_SECT;
	return sect;
}

void lots(int sect, struct pxx *pxx)
{
	double diff;
	
	if (sect == DAY_SECT)
	{
		diff = pxx->dmoon[LONG] - pxx->dsun[LONG];
		pxx->dfor[LONG] = pxx->dasc[LONG] - diff;
		pxx->dspir[LONG] = pxx->dasc[LONG] + diff;
	}
	else // night
	{
		diff = pxx->dmoon[LONG] - pxx->dsun[LONG];
		pxx->dfor[LONG] = pxx->dasc[LONG] + diff;
		pxx->dspir[LONG] = pxx->dasc[LONG] - diff;
	}
	
	pxx->dfor[LONG] = fmod(pxx->dfor[LONG], 360.0);
	if (pxx->dfor[LONG] < 0.0)
		pxx->dfor[LONG]+= 360.0;
	pxx->dspir[LONG] = fmod(pxx->dspir[LONG], 360.0);
	if (pxx->dspir[LONG] < 0.0)
		pxx->dspir[LONG] += 360.0;
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

void zxx_fill(double *z_arr[])
{
	z_arr[ARI][ELEMENT] = FIRE;
}

void pxx_fill(double cusps[], double *p_arr[], double *z_arr[],
struct cdata *cdata, struct pxx *pxx)
{
	int iflag, ipl, iret;
	double xx[6];
	char serr[AS_MAXCH];
	double ascmc[10];
	int ihsy = 'W';
	
	calculate_utc(cdata);
	
	double jd_ut = swe_julday(cdata->utc_year, cdata->utc_mon, 
	cdata->utc_mday, cdata->utc_hour, SE_GREG_CAL);
	
	iflag = SEFLG_SWIEPH | SEFLG_SPEED;
	
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
	
	int calc_flag[RETROCOUNT] = {0};
	int iter[RETROCOUNT] = {0};
	double last_jd = jd_ut;
	
	for (ipl = SE_MERCURY; ipl <= SE_PLUTO; ipl++)
	{
		next_retro_station(jd_ut, p_arr, calc_flag, iter, &last_jd);
		
		if (p_arr[ipl][NEXT_R] <= IS_RETRO)
			p_arr[ipl][RETRO] = 1;
		else
			p_arr[ipl][RETRO] = 0;
		if (p_arr[ipl][NEXT_S] <= STATION_POINT)
			p_arr[ipl][STATION] = STATION_D;
		else if (p_arr[ipl][NEXT_R] <=
		STATION_POINT && p_arr[ipl][NEXT_R] > IS_RETRO)
			p_arr[ipl][STATION] = STATION_R;
		else
			p_arr[ipl][STATION] = 0;
	}
	
	iret = swe_houses_ex(jd_ut, 0, cdata->dlat, cdata->dlon,
	ihsy, cusps, ascmc);
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
	zxx_fill(z_arr);
}


