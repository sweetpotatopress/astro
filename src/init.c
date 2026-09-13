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

void calc_init(double *planet[], double *se)
{
	for (int ipl = SE_MERCURY; ipl <= SE_PLUTO; ++ipl)
		planet[ipl][RET_INIT] = 0;
	se[E_INIT] = 0;
}

void planet_init(double *planet[], int cc, struct pxx **pxx)
{
	double *new_planet[] = {
		pxx[cc]->dsun, pxx[cc]->dmoon,
		pxx[cc]->dmerc, pxx[cc]->dven,
		pxx[cc]->dmars, pxx[cc]->djup,
		pxx[cc]->dsat, pxx[cc]->dura,
		pxx[cc]->dnep, pxx[cc]->dplu,
		pxx[cc]->dmnod, pxx[cc]->dtnod,
		pxx[cc]->dasc, pxx[cc]->dmc,
		pxx[cc]->ddsc, pxx[cc]->dic,
		pxx[cc]->dfor, pxx[cc]->dspir};
		
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

void pxx_init(struct cdata *cdata, struct pxx *pxx, double **planet)
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
	
	eclipse(jd_ut, cdata->le, cdata->se);
	
	iret = swe_houses_ex(jd_ut, 0, cdata->dlat, cdata->dlon,
	'W', cdata->sign_cusp, ascmc);
	if (iret < 0)
		ERR_EXIT("ERR: swe_houses_ex failure");
		
	iret = swe_houses_ex(jd_ut, 0, cdata->dlat, cdata->dlon,
	ihsy, cdata->cusp, ascmc);
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

static void draw_chart(WINDOW *win, struct cdata *cdata, struct pxx *pxx, struct ui *ui, 
double **planet, int **zodiac)
{
	curs_set(0);
	werase(win);
	
	int win_h, win_w;
	getmaxyx(win, win_h, win_w);
	
	int cy = (win_h / 2);
	int cx = (win_w / 2);
	if ((win_w - win_h) > 60)
		cx += 9;
		
	const int radius = (((win_w / 2 < win_h) ? win_w / 2 : win_h) - 5) - ui->roff;
	const int out_r = radius + 4;
	const int in_r = (radius / 2) - 1;
	const int house_r = radius + 4;
	const int zo_r = radius + 3;
	const int pl_r = radius - 5;
	const int as_r = (radius / 2) + 4;
	
	draw_circle(win, out_r, cy, cx, '`');
	draw_circle(win, radius, cy, cx,'.');
	draw_circle(win, in_r, cy, cx, '.');
	
	draw_house(win, cdata, house_r, cy, cx, '`');
	
	zo_pos(win, cdata, pxx, ui, zo_r, cy, cx, zodiac);
	
	planet_pos(win, cdata, ui, planet, zodiac, pl_r, cy, cx);
	
	if (fabs(cdata->dlat) > 1e-6)
		ascmc_pos(win, cdata, planet, zodiac, as_r, cy, cx);
	
	// status bar
	const int bar_end = 20;
	mvwhline(win, 1, win_w - bar_end, '-', COLS);
	mvwvline(win, 0, win_w - bar_end, ':', 1);
	
	mvwprintw(win, 0, win_w - (bar_end - 2), "%d :", ui->cc);
}

void new_chart(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac)
{
	if (setenv("TZ", cdata->timezone, 1) != 0)
		ERR_EXIT("ERR: new_chart setenv");
	tzset();
	
	pxx_init(cdata, pxx, planet);
	draw_chart(ui->main_win, cdata, pxx, ui, planet, zodiac);
	cc_data(ui->main_win, cdata, ui);
	
	if (ui->left_trig > 0)
	{
		left_table(cdata, pxx, ui, planet, zodiac);
		show_panel(ui->left_panel);
	}
	if (ui->right_trig > 0)
	{
		right_table(cdata, ui, planet, zodiac);
		show_panel(ui->right_panel);
	}	
	update_panels();
}
