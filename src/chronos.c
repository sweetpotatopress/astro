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
	z_arr[ARI][RULER] = SE_MARS;
	z_arr[ARI][EXALT] = SE_SUN;
	z_arr[ARI][TRIPLD] = SE_SUN;
	z_arr[ARI][TRIPLN] = SE_JUPITER;
	z_arr[ARI][TRIPLC] = SE_SATURN;
	z_arr[ARI][BOUND0] = SE_JUPITER;
	z_arr[ARI][BOUND1] = SE_VENUS;
	z_arr[ARI][BOUND2] = SE_MERCURY;
	z_arr[ARI][BOUND3] = SE_MARS;
	z_arr[ARI][BOUND4] = SE_SATURN;
	z_arr[ARI][DECAN0] = SE_MARS;
	z_arr[ARI][DECAN1] = SE_SUN;
	z_arr[ARI][DECAN2] = SE_VENUS;
	z_arr[ARI][DETRI] = SE_VENUS;
	z_arr[ARI][FALL] = SE_SATURN;
	
	z_arr[TAU][ELEMENT] = EARTH;
	z_arr[TAU][RULER] = SE_VENUS;
	z_arr[TAU][EXALT] = SE_MOON;
	z_arr[TAU][TRIPLD] = SE_VENUS;
	z_arr[TAU][TRIPLN] = SE_MOON;
	z_arr[TAU][TRIPLC] = SE_MARS;
	z_arr[TAU][BOUND0] = SE_VENUS;
	z_arr[TAU][BOUND1] = SE_MERCURY;
	z_arr[TAU][BOUND2] = SE_JUPITER;
	z_arr[TAU][BOUND3] = SE_SATURN;
	z_arr[TAU][BOUND4] = SE_MARS;
	z_arr[TAU][DECAN0] = SE_MERCURY;
	z_arr[TAU][DECAN1] = SE_MOON;
	z_arr[TAU][DECAN2] = SE_SATURN;
	z_arr[TAU][DETRI] = SE_MARS;
	z_arr[TAU][FALL] = -1.0;
	
	z_arr[GEM][ELEMENT] = AIR;
	z_arr[GEM][RULER] = SE_MERCURY;
	z_arr[GEM][EXALT] = -1.0;
	z_arr[GEM][TRIPLD] = SE_SATURN;
	z_arr[GEM][TRIPLN] = SE_MERCURY;
	z_arr[GEM][TRIPLC] = SE_JUPITER;
	z_arr[GEM][BOUND0] = SE_MERCURY;
	z_arr[GEM][BOUND1] = SE_JUPITER;
	z_arr[GEM][BOUND2] = SE_VENUS;
	z_arr[GEM][BOUND3] = SE_MARS;
	z_arr[GEM][BOUND4] = SE_SATURN;
	z_arr[GEM][DECAN0] = SE_JUPITER;
	z_arr[GEM][DECAN1] = SE_MARS;
	z_arr[GEM][DECAN2] = SE_SUN;
	z_arr[GEM][DETRI] = SE_JUPITER;
	z_arr[GEM][FALL] = -1.0;
	
	z_arr[CAN][ELEMENT] = WATER;
	z_arr[CAN][RULER] = SE_MOON;
	z_arr[CAN][EXALT] = SE_JUPITER;
	z_arr[CAN][TRIPLD] = SE_VENUS;
	z_arr[CAN][TRIPLN] = SE_MARS;
	z_arr[CAN][TRIPLC] = SE_MOON;
	z_arr[CAN][BOUND0] = SE_MARS;
	z_arr[CAN][BOUND1] = SE_VENUS;
	z_arr[CAN][BOUND2] = SE_MERCURY;
	z_arr[CAN][BOUND3] = SE_JUPITER;
	z_arr[CAN][BOUND4] = SE_SATURN;
	z_arr[CAN][DECAN0] = SE_VENUS;
	z_arr[CAN][DECAN1] = SE_MERCURY;
	z_arr[CAN][DECAN2] = SE_MOON;
	z_arr[CAN][DETRI] = SE_SATURN;
	z_arr[CAN][FALL] = SE_MARS;
	
	z_arr[LEO][ELEMENT] = FIRE;
	z_arr[LEO][RULER] = SE_SUN;
	z_arr[LEO][EXALT] = -1.0;
	z_arr[LEO][TRIPLD] = SE_SUN;
	z_arr[LEO][TRIPLN] = SE_JUPITER;
	z_arr[LEO][TRIPLC] = SE_SATURN;
	z_arr[LEO][BOUND0] = SE_JUPITER;
	z_arr[LEO][BOUND1] = SE_VENUS;
	z_arr[LEO][BOUND2] = SE_SATURN;
	z_arr[LEO][BOUND3] = SE_MERCURY;
	z_arr[LEO][BOUND4] = SE_MARS;
	z_arr[LEO][DECAN0] = SE_SATURN;
	z_arr[LEO][DECAN1] = SE_JUPITER;
	z_arr[LEO][DECAN2] = SE_MARS;
	z_arr[LEO][DETRI] = SE_SATURN;
	z_arr[LEO][FALL] = -1.0;
	
	z_arr[VIR][ELEMENT] = EARTH;
	z_arr[VIR][RULER] = SE_MERCURY;
	z_arr[VIR][EXALT] = SE_MERCURY;
	z_arr[VIR][TRIPLD] = SE_VENUS;
	z_arr[VIR][TRIPLN] = SE_MOON;
	z_arr[VIR][TRIPLC] = SE_MARS;
	z_arr[VIR][BOUND0] = SE_MERCURY;
	z_arr[VIR][BOUND1] = SE_VENUS;
	z_arr[VIR][BOUND2] = SE_JUPITER;
	z_arr[VIR][BOUND3] = SE_MARS;
	z_arr[VIR][BOUND4] = SE_SATURN;
	z_arr[VIR][DECAN0] = SE_SUN;
	z_arr[VIR][DECAN1] = SE_VENUS;
	z_arr[VIR][DECAN2] = SE_MERCURY;
	z_arr[VIR][DETRI] = SE_JUPITER;
	z_arr[VIR][FALL] = SE_VENUS;
	
	z_arr[LIB][ELEMENT] = AIR;
	z_arr[LIB][RULER] = SE_VENUS;
	z_arr[LIB][EXALT] = SE_SATURN;
	z_arr[LIB][TRIPLD] = SE_SATURN;
	z_arr[LIB][TRIPLN] = SE_MERCURY;
	z_arr[LIB][TRIPLC] = SE_JUPITER;
	z_arr[LIB][BOUND0] = SE_SATURN;
	z_arr[LIB][BOUND1] = SE_MERCURY;
	z_arr[LIB][BOUND2] = SE_JUPITER;
	z_arr[LIB][BOUND3] = SE_VENUS;
	z_arr[LIB][BOUND4] = SE_MARS;
	z_arr[LIB][DECAN0] = SE_MOON;
	z_arr[LIB][DECAN1] = SE_SATURN;
	z_arr[LIB][DECAN2] = SE_JUPITER;
	z_arr[LIB][DETRI] = SE_MARS;
	z_arr[LIB][FALL] = SE_SUN;
	
	z_arr[SCO][ELEMENT] = WATER;
	z_arr[SCO][RULER] = SE_MARS;
	z_arr[SCO][EXALT] = -1.0;
	z_arr[SCO][TRIPLD] = SE_VENUS;
	z_arr[SCO][TRIPLN] = SE_MARS;
	z_arr[SCO][TRIPLC] = SE_MOON;
	z_arr[SCO][BOUND0] = SE_MARS;
	z_arr[SCO][BOUND1] = SE_VENUS;
	z_arr[SCO][BOUND2] = SE_MERCURY;
	z_arr[SCO][BOUND3] = SE_JUPITER;
	z_arr[SCO][BOUND4] = SE_SATURN;
	z_arr[SCO][DECAN0] = SE_MARS;
	z_arr[SCO][DECAN1] = SE_SUN;
	z_arr[SCO][DECAN2] = SE_VENUS;
	z_arr[SCO][DETRI] = SE_VENUS;
	z_arr[SCO][FALL] = SE_MOON;
	
	z_arr[SAG][ELEMENT] = FIRE;
	z_arr[SAG][RULER] = SE_JUPITER;
	z_arr[SAG][EXALT] = -1.0;
	z_arr[SAG][TRIPLD] = SE_SUN;
	z_arr[SAG][TRIPLN] = SE_JUPITER;
	z_arr[SAG][TRIPLC] = SE_SATURN;
	z_arr[SAG][BOUND0] = SE_JUPITER;
	z_arr[SAG][BOUND1] = SE_VENUS;
	z_arr[SAG][BOUND2] = SE_MERCURY;
	z_arr[SAG][BOUND3] = SE_SATURN;
	z_arr[SAG][BOUND4] = SE_MARS;
	z_arr[SAG][DECAN0] = SE_MERCURY;
	z_arr[SAG][DECAN1] = SE_MOON;
	z_arr[SAG][DECAN2] = SE_SATURN;
	z_arr[SAG][DETRI] = SE_MERCURY;
	z_arr[SAG][FALL] = -1.0;
	
	z_arr[CAP][ELEMENT] = EARTH;
	z_arr[CAP][RULER] = SE_SATURN;
	z_arr[CAP][EXALT] = SE_MARS;
	z_arr[CAP][TRIPLD] = SE_VENUS;
	z_arr[CAP][TRIPLN] = SE_MOON;
	z_arr[CAP][TRIPLC] = SE_MARS;
	z_arr[CAP][BOUND0] = SE_MERCURY;
	z_arr[CAP][BOUND1] = SE_JUPITER;
	z_arr[CAP][BOUND2] = SE_VENUS;
	z_arr[CAP][BOUND3] = SE_SATURN;
	z_arr[CAP][BOUND4] = SE_MARS;
	z_arr[CAP][DECAN0] = SE_JUPITER;
	z_arr[CAP][DECAN1] = SE_MARS;
	z_arr[CAP][DECAN2] = SE_SUN;
	z_arr[CAP][DETRI] = SE_MOON;
	z_arr[CAP][FALL] = SE_JUPITER;
	
	z_arr[AQU][ELEMENT] = AIR;
	z_arr[AQU][RULER] = SE_SATURN;
	z_arr[AQU][EXALT] = -1.0;
	z_arr[AQU][TRIPLD] = SE_SATURN;
	z_arr[AQU][TRIPLN] = SE_MERCURY;
	z_arr[AQU][TRIPLC] = SE_JUPITER;
	z_arr[AQU][BOUND0] = SE_MERCURY;
	z_arr[AQU][BOUND1] = SE_VENUS;
	z_arr[AQU][BOUND2] = SE_JUPITER;
	z_arr[AQU][BOUND3] = SE_MARS;
	z_arr[AQU][BOUND4] = SE_SATURN;
	z_arr[AQU][DECAN0] = SE_VENUS;
	z_arr[AQU][DECAN1] = SE_MERCURY;
	z_arr[AQU][DECAN2] = SE_MOON;
	z_arr[AQU][DETRI] = SE_SUN;
	z_arr[AQU][FALL] = -1.0;
	
	z_arr[PIS][ELEMENT] = WATER;
	z_arr[PIS][RULER] = SE_JUPITER;
	z_arr[PIS][EXALT] = SE_VENUS;
	z_arr[PIS][TRIPLD] = SE_VENUS;
	z_arr[PIS][TRIPLN] = SE_MARS;
	z_arr[PIS][TRIPLC] = SE_MOON;
	z_arr[PIS][BOUND0] = SE_VENUS;
	z_arr[PIS][BOUND1] = SE_JUPITER;
	z_arr[PIS][BOUND2] = SE_MERCURY;
	z_arr[PIS][BOUND3] = SE_MARS;
	z_arr[PIS][BOUND4] = SE_SATURN;
	z_arr[PIS][DECAN0] = SE_SATURN;
	z_arr[PIS][DECAN1] = SE_JUPITER;
	z_arr[PIS][DECAN2] = SE_MARS;
	z_arr[PIS][DETRI] = SE_MERCURY;
	z_arr[PIS][FALL] = SE_MERCURY;
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
	z_arr[ARI][JOY] = 0; // temp placeholder
}


