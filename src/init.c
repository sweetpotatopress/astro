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
 
#include <stdlib.h>
#include "swephexp.h"
#include "astro.h"
#include "table.h"
#include "chronos.h"
#include "anim.h"
#include "draw.h"

void pxx_init(double cusp[], double sign_cusp[],
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
}

void draw_chart(WINDOW *win, double cusp[], double sign_cusp[], double *planet[], int *zodiac[],
struct pxx *pxx, struct cdata *cdata,  const char *pl_sym[], const char *zo_sym[])
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
	int bar_end = 15;
	mvwhline(win, 1, COLS - bar_end, '.', COLS);
	mvwvline(win, 0, COLS - bar_end, '.', 2);
}

void new_chart(NEW_CHART_PARAM())
{
	pxx_init(cusp, sign_cusp, luna_eclipse, sol_eclipse,
	planet, cdata, pxx);
	draw_chart(main_win, cusp, sign_cusp, planet, zodiac, pxx, cdata,
	pl_sym, zo_sym);
	cur_chart_data(main_win, io, cdata);
	
	if (*left_trig > 0)
	{
		left_table(left_win, planet, zodiac, pxx,
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
