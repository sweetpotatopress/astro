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

int months(int month, int year)
{
	int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	if (month == 2)
		if (((year + 1900) % 4 == 0 && (year + 1900) % 100 != 0) || 
		((year + 1900) % 400 == 0))
			return 29;
	return days[month];
}

void retro_calc(struct pxx *pxx, double jul_day_UT, int iter[], int ipl)
{
	int iflag;
	double xx[6];
	char serr[AS_MAXCH];
	
	double *p_arr[] = {
	NULL, NULL,
	pxx->dmerc, pxx->dven,
	pxx->dmars, pxx->djup,
	pxx->dsat, pxx->dura,
	pxx->dnep, pxx->dplu};
	
	iflag = SEFLG_SWIEPH | SEFLG_SPEED;
	
	double julday_copy = jul_day_UT;
	
	while(p_arr[ipl][LONG_S] > 0.0)
	{
		swe_calc_ut(julday_copy, ipl, iflag, xx, serr);
		julday_copy += PARSECOUNT;
		p_arr[ipl][LONG_S] = xx[LONG_S];
		p_arr[ipl][NEXT_R] = (int)julday_copy - jul_day_UT;
		if (p_arr[ipl][NEXT_R] < STATION_R)
			p_arr[ipl][NEXT_R] = IS_RETRO;
	}
	
	while(p_arr[ipl][LONG_S] <= 0.0)
	{
		swe_calc_ut(julday_copy, ipl, iflag, xx, serr);
		julday_copy += PARSECOUNT;
		p_arr[ipl][LONG_S] = xx[LONG_S];
		p_arr[ipl][NEXT_S] = (int)(jul_day_UT - julday_copy);
	}
	iter[ipl] = 0;
}

void next_retro_station(struct pxx *pxx, double jul_day_UT)
{
	int ipl;
	
	static int calc_flag[RETROCOUNT] = {0};
	static double last_jd;
	static int iter[RETROCOUNT] = {0};
	
	double *p_arr[] = {
	NULL, NULL,
	pxx->dmerc, pxx->dven,
	pxx->dmars, pxx->djup,
	pxx->dsat, pxx->dura,
	pxx->dnep, pxx->dplu};
	
	for (ipl = SE_MERCURY; ipl <= SE_PLUTO; ipl++)
	{
		if (calc_flag[ipl] == 0)
		{
			retro_calc(pxx, jul_day_UT, iter, ipl);
			calc_flag[ipl] = 1;
			
			if (p_arr[ipl][LONG_S] < 0.0)
				p_arr[ipl][NEXT_R] = IS_RETRO;
				
			if (ipl == SE_MERCURY &&
			p_arr[ipl][LONG_S] < 0.1)
				p_arr[ipl][NEXT_R] = IS_RETRO;
		}
		else if (fabs(last_jd - jul_day_UT) >= 1.0)
		{
			double offset = (int)fabs(last_jd - jul_day_UT);
			if (offset > PARSECOUNT)
				break;
			
			if (last_jd < jul_day_UT)
			{
				p_arr[ipl][NEXT_S] += (int)offset;
				p_arr[ipl][NEXT_R] -= (int)offset;
				if (p_arr[ipl][NEXT_R] <= 0.0)
					p_arr[ipl][NEXT_R] = IS_RETRO;
			}
			else if (last_jd > jul_day_UT)
			{
				p_arr[ipl][NEXT_S] += (int)offset;
				if (p_arr[ipl][NEXT_R] > IS_RETRO)
					p_arr[ipl][NEXT_R] += (int)offset;
			}

			if (p_arr[ipl][LONG_S] < 0.0)
				p_arr[ipl][NEXT_R] = IS_RETRO;
				
			if (ipl == SE_MERCURY &&
			p_arr[SE_MERCURY][LONG_S] < 0.1)
				p_arr[ipl][NEXT_R] = IS_RETRO;
		}
		
		if ((p_arr[ipl][NEXT_R] < 50.0 && iter[ipl] >= ITERMAX) ||
		(p_arr[ipl][NEXT_R] > 90.0 && iter[ipl] >= ITERMAX))
			retro_calc(pxx, jul_day_UT, iter, ipl);
			
		if (p_arr[ipl][NEXT_R] > -0.0001 && p_arr[ipl][NEXT_R] < 0.0001)
			retro_calc(pxx, jul_day_UT, iter, ipl);
			
		iter[ipl]++;
	}
	last_jd = jul_day_UT;
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

void pxx_fill(double cusps[], struct cdata *cdata, struct pxx *pxx)
{
	int iflag, ipl;
	double xx[6];
	char serr[AS_MAXCH];
	double ascmc[10];
	int ihsy = 'W';
	
	int iret;
	size_t i;
	
	double *p_arr[] = {
	pxx->dsun, pxx->dmoon,
	pxx->dmerc, pxx->dven,
	pxx->dmars, pxx->djup,
	pxx->dsat, pxx->dura,
	pxx->dnep, pxx->dplu,
	pxx->dmnod, pxx->dtnod,
	pxx->dasc, pxx->dmc,
	pxx->ddsc, pxx->dic,
	pxx->dfor, pxx->dspir};
	
	calculate_utc(cdata);
	
	double jul_day_UT = swe_julday(cdata->utc_year, cdata->utc_mon, 
	cdata->utc_mday, cdata->utc_hour, SE_GREG_CAL);
	
	iflag = SEFLG_SWIEPH | SEFLG_SPEED;
	for (ipl = SE_MERCURY; ipl <= SE_PLUTO; ipl++)
		next_retro_station(pxx, jul_day_UT);
	
	for (ipl = SE_SUN, i = 0; ipl <= SE_TRUE_NODE; ipl++, i++)
	{
		iret = swe_calc_ut(jul_day_UT, ipl, iflag, xx, serr);
		if (iret < 0) 
			ERR_EXIT("ERR: swe_calc_ut failure");
			
		p_arr[i][LONG] = xx[LONG];
		p_arr[i][LAT] = xx[LAT];
		p_arr[i][DIST] = xx[DIST];
		p_arr[i][LONG_S] = xx[LONG_S];
		p_arr[i][LAT_S] = xx[LAT_S];
		p_arr[i][DIST_S] = xx[DIST_S];
		if (ipl >= SE_MERCURY && ipl <= SE_PLUTO)
		{
			if (p_arr[i][NEXT_R] <= IS_RETRO)
				p_arr[i][RETRO] = 1;
			else
				p_arr[i][RETRO] = 0;
			if (fabs(p_arr[i][NEXT_S]) <= 7)
				p_arr[i][STATION] = STATION_D;
			else if (p_arr[i][NEXT_R] <= 7 && p_arr[i][NEXT_R] > 0.5)
				p_arr[i][STATION] = STATION_R;
			else
				p_arr[i][STATION] = 0;
		}
	}
	
	iret = swe_houses_ex(jul_day_UT, 0, cdata->dlat, cdata->dlon,
	ihsy, cusps, ascmc);
	if (iret < 0)
		ERR_EXIT("ERR: swe_houses_ex failure");
	
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
}


	
		
	 
