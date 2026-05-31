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

void check_dst(struct cdata *cdata)
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
	
	struct tm *utc_tm = gmtime(&t);
	
	int offset_hours = tm_in.tm_hour - utc_tm->tm_hour;
	
	if (tm_in.tm_mday != utc_tm->tm_mday)
	{
		if (tm_in.tm_mday > utc_tm->tm_mday)
			offset_hours += 24;
		else
			offset_hours -= 24;
	}
	
	cdata->utc_off = offset_hours;
}

void calculate_utc(struct cdata *cdata, int *day_offset)
{
	check_dst(cdata);
	
	double utc_offset = (double)cdata->utc_off;
	
	double min = (double)cdata->tm_min / 60;
	double sec = (double)cdata->tm_sec / 3600.0;
	
	double utc_hour = ((double)(cdata->tm_hour - utc_offset) + min  ) + sec;
	
	if((utc_hour >= 24.0))
	{
		utc_hour -= 24.0;
		++cdata->tm_mday;
		*day_offset -= 1;
	}
	else if((utc_hour <= 0))
	{
		utc_hour += 24.0;
		--cdata->tm_mday;
		*day_offset += 1;
	}
	
	cdata->utc_hour = utc_hour; 
}

void retro_calc(struct pxx *pxx, double jul_day_UT, int iter[], int ipl)
{
	int iflag;
	double xx[6];
	char serr[AS_MAXCH];
	
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
	
	iflag = SEFLG_SWIEPH | SEFLG_SPEED;
	
	double julday_copy = jul_day_UT;
	
	while(p_arr[ipl][LONG_S] > 0.01)
	{
		swe_calc_ut(julday_copy, ipl, iflag, xx, serr);
		julday_copy += 2;
		p_arr[ipl][LONG_S] = xx[LONG_S];
		p_arr[ipl][NEXT_R] = julday_copy - jul_day_UT;
		if(p_arr[ipl][NEXT_R] <= 2)
			p_arr[ipl][NEXT_R] = 0;
	}
	
	while(p_arr[ipl][LONG_S] <= 0.01)
	{
		swe_calc_ut(julday_copy, ipl, iflag, xx, serr);
		julday_copy -= 2;
		p_arr[ipl][LONG_S] = xx[LONG_S];
		p_arr[ipl][LAST_R] = jul_day_UT - julday_copy;
	}
	iter[ipl] = 0;
}

void retro_days(struct pxx *pxx, double jul_day_UT)
{
	int ipl;
	
	static int calc_flag[SE_PLUTO + 1] = {0};
	static double last_jd;
	static int iter[10] = {0};
	
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
		}
		else if (fabs(last_jd - jul_day_UT) >= 1)
		{
			double offset = fabs(last_jd - jul_day_UT);
			if (offset > 3)
				break;
			
			if (last_jd < jul_day_UT)
			{
				p_arr[ipl][LAST_R] += (int)offset;
				p_arr[ipl][NEXT_R] -= (int)offset;
			}
			else if (last_jd > jul_day_UT)
			{
				p_arr[ipl][LAST_R] -= (int)offset;
				p_arr[ipl][NEXT_R] += (int)offset;
			}
		}
		
		if ((p_arr[ipl][NEXT_R] < 100 && iter[ipl] >= 10) ||
		(p_arr[ipl][NEXT_R] > 90 && iter[ipl] >= 20))
			retro_calc(pxx, jul_day_UT, iter, ipl);
			
		if (p_arr[ipl][NEXT_R] > -0.01 && p_arr[ipl][NEXT_R] < 0.01)
			retro_calc(pxx, jul_day_UT, iter, ipl);
			
		iter[ipl]++;
	}
	last_jd = jul_day_UT;
}
	
void pxx_fill(double cusps[], struct cdata *cdata, struct pxx *pxx)
{
	int iflag, ipl;
	double xx[6];
	char serr[AS_MAXCH];
	double ascmc[10];
	int ihsy = 'W';
	
	int day_offset = 0;
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
	
	calculate_utc(cdata, &day_offset);
	
	double jul_day_UT = swe_julday(cdata->tm_year, cdata->tm_mon, 
	cdata->tm_mday, cdata->utc_hour, SE_GREG_CAL);
	
	// corrects cdata->utc_hour offset from calculate_utc()
	cdata->tm_mday += day_offset;

	iflag = SEFLG_SWIEPH | SEFLG_SPEED;
	for (ipl = SE_MERCURY; ipl <= SE_PLUTO; ipl++)
		retro_days(pxx, jul_day_UT);
	ipl = 0;
	
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
		if (p_arr[i][LONG_S] < -0.005)
			p_arr[i][RETRO] = 1;
		else
			p_arr[i][RETRO] = 0;
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


	
		
	 
