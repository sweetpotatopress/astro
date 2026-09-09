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
 
#include <stdlib.h>
#include <time.h>
#include "swephexp.h"
#include "astro.h"
#include "table.h"
#include "chronos.h"
#include "anim.h"
#include "draw.h"

void cdata_init(struct cdata *cdata)
{
	memset(cdata->chart_name, 0, MAXBUF);
	memset(cdata->city, 0, MAXBUF);
	memset(cdata->country, 0, MAXBUF);
	memset(cdata->state, 0, MAXBUF);
	cdata->isdst = -1;
}

void calc_init(double *planet[], double *sol_eclipse)
{
	for (int ipl = SE_MERCURY; ipl <= SE_PLUTO; ++ipl)
		planet[ipl][RET_INIT] = 0;
	sol_eclipse[E_INIT] = 0;
}

void planet_init(double *planet[], int cur_chart, struct pxx **pxx)
{
	double *new_planet[] = {
		pxx[cur_chart]->dsun, pxx[cur_chart]->dmoon,
		pxx[cur_chart]->dmerc, pxx[cur_chart]->dven,
		pxx[cur_chart]->dmars, pxx[cur_chart]->djup,
		pxx[cur_chart]->dsat, pxx[cur_chart]->dura,
		pxx[cur_chart]->dnep, pxx[cur_chart]->dplu,
		pxx[cur_chart]->dmnod, pxx[cur_chart]->dtnod,
		pxx[cur_chart]->dasc, pxx[cur_chart]->dmc,
		pxx[cur_chart]->ddsc, pxx[cur_chart]->dic,
		pxx[cur_chart]->dfor, pxx[cur_chart]->dspir};
		
	memcpy(planet, new_planet, sizeof(new_planet));
}

void zxx_init(int *zodiac[])
{ /* element, ruler, exalt, triplicity d,n,c,
	bound 0-4, decan 0-2, detri, fall */
	
	const int ari[] = 
	{ FIRE, SE_MARS, SE_SUN, SE_SUN,
	SE_JUPITER, SE_SATURN, SE_JUPITER, SE_VENUS,
	SE_MERCURY, SE_MARS, SE_SATURN, SE_MARS,
	SE_SUN, SE_VENUS, SE_VENUS, SE_SATURN };

	const int tau[] = 
	{ EARTH, SE_VENUS, SE_MOON, SE_VENUS,
	SE_MOON, SE_MARS, SE_VENUS, SE_MERCURY,
	SE_JUPITER, SE_SATURN, SE_MARS, SE_MERCURY,
	SE_MOON, SE_SATURN, SE_MARS, EMPTY };
	
	const int gem[] =
	{ AIR, SE_MERCURY, EMPTY, SE_SATURN,
	SE_MERCURY, SE_JUPITER, SE_MERCURY, SE_JUPITER,
	SE_VENUS, SE_MARS, SE_SATURN, SE_JUPITER,
	SE_MARS, SE_SUN, SE_JUPITER, EMPTY };

	const int can[] = 
	{ WATER, SE_MOON, SE_JUPITER, SE_VENUS,
	SE_MARS, SE_MOON, SE_MARS, SE_VENUS,
	SE_MERCURY, SE_JUPITER, SE_SATURN, SE_VENUS,
	SE_MERCURY, SE_MOON, SE_SATURN, SE_MARS };
	
	const int leo[] =
	{ FIRE, SE_SUN, EMPTY, SE_SUN,
	SE_JUPITER, SE_SATURN, SE_JUPITER, SE_VENUS,
	SE_SATURN, SE_MERCURY, SE_MARS, SE_SATURN,
	SE_JUPITER, SE_MARS, SE_SATURN, EMPTY };
	
	const int vir[] =
	{ EARTH, SE_MERCURY, SE_MERCURY, SE_VENUS,
	SE_MOON, SE_MARS, SE_MERCURY, SE_VENUS,
	SE_JUPITER, SE_MARS, SE_SATURN, SE_SUN,
	SE_VENUS, SE_MERCURY, SE_JUPITER, SE_VENUS };
	
	const int lib[] = 
	{ AIR, SE_VENUS, SE_SATURN, SE_SATURN,
	SE_MERCURY, SE_JUPITER, SE_SATURN, SE_MERCURY,
	SE_JUPITER, SE_VENUS, SE_MARS, SE_MOON,
	SE_SATURN, SE_JUPITER, SE_MARS, SE_SUN };
	
	const int sco[] =
	{ WATER, SE_MARS, EMPTY, SE_VENUS,
	SE_MARS, SE_MOON, SE_MARS, SE_VENUS,
	SE_MERCURY, SE_JUPITER, SE_SATURN, SE_MARS,
	SE_SUN, SE_VENUS, SE_VENUS, SE_MOON };
	
	const int sag[] =
	{ FIRE, SE_JUPITER, EMPTY, SE_SUN,
	SE_JUPITER, SE_SATURN, SE_JUPITER, SE_VENUS,
	SE_MERCURY, SE_SATURN, SE_MARS, SE_MERCURY,
	SE_MOON, SE_SATURN, SE_MERCURY, EMPTY };
	
	const int cap[] =
	{ EARTH, SE_SATURN, SE_MARS, SE_VENUS,
	SE_MOON, SE_MARS, SE_MERCURY, SE_JUPITER,
	SE_VENUS, SE_SATURN, SE_MARS, SE_JUPITER,
	SE_MARS, SE_SUN, SE_MOON, SE_JUPITER };
	
	const int aqu[] =
	{ AIR, SE_SATURN, EMPTY, SE_SATURN,
	SE_MERCURY, SE_JUPITER, SE_MERCURY, SE_VENUS,
	SE_JUPITER, SE_MARS, SE_SATURN, SE_VENUS,
	SE_MERCURY, SE_MOON, SE_SUN, EMPTY };
	
	const int pis[] =
	{ WATER, SE_JUPITER, SE_VENUS, SE_VENUS,
	SE_MARS, SE_MOON, SE_VENUS, SE_JUPITER,
	SE_MERCURY, SE_MARS, SE_SATURN, SE_SATURN,
	SE_JUPITER, SE_MARS, SE_MERCURY, SE_MERCURY }; 
	
	const int *zodia[] = { 0,
	ari, tau, gem, can,
	leo, vir, lib, sco,
	sag, cap, aqu, pis};

	int d, z;
	for (z = ARI; z < ZMAX; ++z)
		for(d = ELEMENT; d <= FALL; ++d)
			zodiac[z][d] = zodia[z][d];
}

static void pxx_init(double cusp[], double sign_cusp[],
double *luna_eclipse, double *sol_eclipse, double *planet[],
struct cdata *cdata, struct pxx *pxx)
{
	int iflag = SEFLG_SWIEPH | SEFLG_SPEED;
	int ipl, iret;
	double xx[6];
	char serr[AS_MAXCH];
	double ascmc[10];
	int ihsy = 'W';
	
	calculate_utc(cdata);
	weekday_check(cdata);
	
	double jd_ut = swe_julday(cdata->utc_year, cdata->utc_mon, 
	cdata->utc_mday, cdata->utc_hour, SE_GREG_CAL);
	
	for (ipl = SE_SUN; ipl <= SE_TRUE_NODE; ipl++)
	{
		iret = swe_calc_ut(jd_ut, ipl, iflag, xx, serr);
		if (iret < 0) 
			ERR_EXIT("ERR: swe_calc_ut failure");
			
		planet[ipl][LONG] = xx[LONG];
		planet[ipl][LAT] = xx[LAT];
		planet[ipl][DIST] = xx[DIST];
		planet[ipl][LONG_S] = xx[LONG_S];
		planet[ipl][LAT_S] = xx[LAT_S];
		planet[ipl][DIST_S] = xx[DIST_S];
	}

	retro_station(jd_ut, planet);
	
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
	planet[SE_MEAN_NODE][LONG] = (planet[SE_TRUE_NODE][LONG] + 180);
	if (planet[SE_MEAN_NODE][LONG] >= 360)
		planet[SE_MEAN_NODE][LONG] -= 360;
	
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
	
	lots(pxx);
	
	// seperate degree and minutes
	for (ipl = SE_SUN; ipl < SPXXMAX; ++ipl)
	{
		planet[ipl][DEGREE] = (int)planet[ipl][LONG] % 30;
		planet[ipl][MIN] = (int)((planet[ipl][LONG] - (int)planet[ipl][LONG]) * 60);
		
		planet[ipl][DEGREE_S] = planet[ipl][LONG_S];
		planet[ipl][MIN_S] = (fabs(planet[ipl][LONG_S] - (int)planet[ipl][LONG_S]) * 60);
		if (planet[ipl][LONG_S] < 0)
			planet[ipl][DEGREE_S] *= -1;
	}
	
	// moonphase
	double elongation = pxx->dmoon[LONG] - pxx->dsun[LONG];
	
	while (elongation < 0)
		elongation += 360;
	while (elongation >= 360)
		elongation -= 360;
		
	int phase = (int)(elongation / 45);
	if (phase > 7)
		phase = 7;
	cdata->moonphase = phase;
}

static void draw_chart(WINDOW *win, double cusp[], double sign_cusp[], double *planet[], int *zodiac[],
struct pxx *pxx, struct cdata *cdata,  const char *pl_sym[], const char *zo_sym[], int cur_chart)
{
	curs_set(0);
	werase(win);
	int radius = ((COLS / 2 < LINES) ? COLS / 2 : LINES) - 5;
	
	int offsetx = 0;
	if ((COLS - LINES) > 60)
		offsetx += 9;
	else
		offsetx = 0;
		
	int centery = (LINES / 2);
	int centerx = (COLS / 2) + offsetx;
	
	// zo
	draw_circle(win, radius + 4, centery, centerx, '`');
	// out
	draw_circle(win, radius, centery, centerx,'.');
	// in
	draw_circle(win, (radius / 2) - 1, centery, centerx, '.');
	
	draw_house(win, cusp, radius + 4, centery, centerx, '`');
	
	zo_pos(win, sign_cusp, radius + 3, centery, centerx, pxx,
	zo_sym, zodiac);
	
	planet_pos(win, sign_cusp, planet, zodiac,
	radius - 5, centery, centerx, pl_sym);
	
	if (fabs(cdata->dlat) > 1e-6)
		ascmc_pos(win, sign_cusp, planet, zodiac,
		(radius / 2) + 4, centery, centerx);
	
	// status bar
	int bar_end = 20;
	mvwhline(win, 1, COLS - bar_end, '-', COLS);
	mvwvline(win, 0, COLS - bar_end, ':', 1);
	
	mvwprintw(win, 0, COLS - (bar_end - 2), "%d :", cur_chart);
}

void new_chart(NEW_CHART_PARAM())
{
	if (setenv("TZ", cdata->timezone, 1) != 0)
		ERR_EXIT("ERR: new_chart setenv");
	tzset();
	
	pxx_init(cusp, sign_cusp, luna_eclipse, sol_eclipse,
	planet, cdata, pxx);
	draw_chart(main_win, cusp, sign_cusp, planet, zodiac, pxx, cdata,
	pl_sym, zo_sym, cur_chart);
	cur_chart_data(main_win, cdata);
	
	if (*left_trig > 0)
	{
		left_table(left_win, planet, zodiac, pxx, cdata,
		pl_sym, zo_sym, moon);
		show_panel(*left_panel);
	}
	if (*right_trig > 0)
	{
		right_table(right_win, luna_eclipse, sol_eclipse,
		planet, zodiac, zo_sym, pl_sym);
		show_panel(*right_panel);
	}	
	update_panels();
}
