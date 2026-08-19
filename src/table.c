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

#include "swephexp.h"
#include "astro.h"
#include "chronos.h"
#include "draw.h"
#include "table.h"

void zxx_init(int *z_arr[])
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
			z_arr[z][d] = zodia[z][d];
}

static int bound_check(int sign, int degree)
{
	const int ari[] =
	{ 5, 11, 19, 24, 29 };
	const int tau[] =
	{ 7, 13, 21, 26, 29 };
	const int gem[] =
	{ 5, 11, 16, 24, 29 };
	const int can[] =
	{ 6, 12, 18, 25, 29 };
	const int leo[] =
	{ 5, 10, 17, 23, 29 };
	const int vir[] =
	{ 6, 16, 20, 27, 29 };
	const int lib[] =
	{ 5, 10, 18, 25, 29 };
	const int sco[] =
	{ 6, 10, 18, 23, 29 };
	const int sag[] =
	{ 11, 16, 20, 25, 29 };
	const int cap[] =
	{ 6, 13, 21, 25, 29 };
	const int aqu[] = 
	{ 6, 12, 19, 24, 29 };
	const int pis[] = 
	{ 11, 15, 18, 27, 29 };
	
	const int *zodia[] = { 0,
	ari, tau, gem, can,
	leo, vir, lib, sco,
	sag, cap, aqu, pis };
	
	for (int i = 0; i < 5; ++i)
		if (degree <= zodia[sign][i])
			return i + BOUND0;
	return -1;
}

static void dignity_check(int *z_arr[], double *p_arr[], int result[PLMAX][MAXZXX], struct pxx *pxx)
{
	int planet = 0;
	for (; planet < PLMAX; ++planet)
	{
		int sign = (int)(p_arr[planet][LONG] / 30) + 1;
		int degree = (int)p_arr[planet][DEGREE];
		
		result[planet][RULER] = z_arr[sign][RULER];
		result[planet][EXALT] = z_arr[sign][EXALT];
		result[planet][FALL] = z_arr[sign][FALL];
		result[planet][DETRI] = z_arr[sign][DETRI];
		
		int chart_sect = sect(pxx);
		if (chart_sect == DAY_SECT)
			result[planet][TRIPLD] = z_arr[sign][TRIPLD];
		else
			result[planet][TRIPLD] = z_arr[sign][TRIPLN];
			
		result[planet][BOUND0] = z_arr[sign][bound_check(sign, degree)];
		
		if (degree <= 9)
			result[planet][DECAN0] = z_arr[sign][DECAN0];
		else if (degree > 9 && degree <= 19)
			result[planet][DECAN0] = z_arr[sign][DECAN1];
		else if (degree > 19)
			result[planet][DECAN0] = z_arr[sign][DECAN2];
		else // error
			result[planet][DECAN0] = EMPTY;
	}
}

static int moon_phase(struct pxx *pxx)
{
	double elongation = pxx->dmoon[LONG] - pxx->dsun[LONG];
	
	while (elongation < 0)
		elongation += 360;
	while (elongation >= 360)
		elongation -= 360;
		
	int phase = (int)(elongation / 45);
	if (phase > 7)
		phase = 7;
		
	return phase;
}

void planet_table(WINDOW *planet_win, double *p_arr[], int *z_arr[], struct pxx *pxx, 
const char *pl_sym[], const char *zo_sym[], const char *moon[])
{
	const char *name[17] = { 
	"su", "mo", "me", "ve",
	"ma", "ju", "sa", "ur",
	"ne", "pl", "so", "no",
	"as", "mc", "ds", "ic",
	"  "};
	
	int p_count = 18;
	
	mvwin(planet_win, 0, 0);
	wresize(planet_win, 44, 33);
	
	werase(planet_win);
	
	int starty = 1, startx = 2;
	int j = 0;
	
	for (int i = 0; i < p_count; ++i)
	{
		int sign = ((int)p_arr[i][LONG] / 30) + 1;
		
		int full_deg = (int)p_arr[i][LONG];
		int deg = (int)p_arr[i][LONG] % 30;
		int minute = (int)((p_arr[i][LONG] - (int)p_arr[i][LONG]) * 60);
		
		if (i < 12) // sun -> north node 
		{
			char buff[MAXBUF];
			
			snprintf(buff, sizeof(buff),
			"%-3s %3d.%02d : %6s %02d*%02d`",
			name[i], full_deg, minute,
			pl_sym[i], deg, minute);
			
			mvwprintw(planet_win, starty, startx, "%s ", buff);
			
			if (p_arr[i][RETRO] > 0)
			{
				wattron(planet_win, COLOR_PAIR(FIRE));
				mvwprintw(planet_win, starty, startx + 12, "r");
				wattroff(planet_win, COLOR_PAIR(FIRE));
			}
				
			if ((int)p_arr[i][STATION] == STATION_R)
			{
				wattron(planet_win, COLOR_PAIR(EARTH));
				mvwaddstr(planet_win, starty, startx + 12, "sr");
				wattroff(planet_win, COLOR_PAIR(EARTH));
			}
			else if ((int)p_arr[i][STATION] == STATION_D)
			{
				wattron(planet_win, COLOR_PAIR(EARTH));
				mvwaddstr(planet_win, starty, startx + 12, "sd");
				wattroff(planet_win, COLOR_PAIR(EARTH));
			}
			
			int color_x = startx + (int)strlen(buff) + 1;
			zodiac_color(planet_win, starty, color_x, sign, zo_sym, z_arr);
			
			starty += 1;
		}
		
		else if (i >= 12) // asc -> ic
		{
			if (i == 12 || i == 16)
			{
				mvwprintw(planet_win, starty, startx,
				"------------------------------");
				starty += 1;
			}
			
			const char *points[] = {
			"as", "mc", "ds", "ic", "fortune", "spirit"};
			
			char point_buff[MAXBUF];
			
			snprintf(point_buff, sizeof(point_buff),
			"%-10s %3d.%02d : %02d*%02d`",
			points[j], full_deg, minute, deg, minute);
			
			mvwprintw(planet_win, starty, startx, "%s", point_buff);
			
			int color_x = startx + (int)strlen(point_buff) + 1;
			zodiac_color(planet_win, starty, color_x, sign, zo_sym, z_arr);
	
			starty += 1;
			j++;
		}
	}
	int planet = 0;
	
	mvwprintw(planet_win, starty, startx,
	"------------------------------");
	++starty;
			
	mvwprintw(planet_win, starty, startx, "pl:rul:exa:tri:bou:dec:det:fal:");
	++starty;
	mvwprintw(planet_win, starty, startx,
	"----------------------------xx");
	++starty;
	
	while (planet < PLMAX)
	{
		int result[PLMAX][MAXZXX] = {0};
		dignity_check(z_arr, p_arr, result, pxx);
	
		int dig[] = { RULER, EXALT, TRIPLD,
		BOUND0, DECAN0, DETRI, FALL };
		
		if (planet == 12)
		{
			mvwprintw(planet_win, starty, startx,
			"------------------------------");
			++starty;
		}
		
		mvwprintw(planet_win, starty, startx, "%-2s:", 
		name[planet]);
		startx += 4;
		for (int i = 0; i < 7; ++i)
		{
			if (strcmp(name[planet], name[result[planet][dig[i]]]) == 0 && i < 5)
			{
				wattron(planet_win, COLOR_PAIR(EARTH));
				mvwprintw(planet_win, starty, startx, "%-2s", 
				name[result[planet][dig[i]]]);
				wattroff(planet_win, COLOR_PAIR(EARTH));
				
				startx += 2;
				mvwprintw(planet_win, starty, startx, ":");
				startx += 2;
			}
			else if (strcmp(name[planet], name[result[planet][dig[i]]]) == 0 && i >= 5)
			{
				wattron(planet_win, COLOR_PAIR(FIRE));
				mvwprintw(planet_win, starty, startx, "%-2s", 
				name[result[planet][dig[i]]]);
				wattroff(planet_win, COLOR_PAIR(FIRE));
				
				startx += 2;
				mvwprintw(planet_win, starty, startx, ":");
				startx += 2;
			}
			else
			{
				mvwprintw(planet_win, starty, startx, "%-2s:", 
				name[result[planet][dig[i]]]);
				startx += 4;
			}
		}
		startx = 2;
		++starty;
		++planet;
	}
	mvwprintw(planet_win, starty, startx,
	"------------------------------");
	
	++starty;
	mvwprintw(planet_win, starty, startx, 
	"moon phase: %s", moon[moon_phase(pxx)]);
}

void retro_table(WINDOW *retro_win,
double *luna_eclipse, double *sol_eclipse,
double *p_arr[], int *z_arr[],
const char *zo_sym[], const char *pl_sym[])
{
	size_t p_count = 10;
	
	mvwin(retro_win, LINES - 11, COLS - 24);
	wresize(retro_win, 11, 24);
	
	werase(retro_win);
	
	mvwprintw(retro_win, 0, 0, "(()");
	zodiac_color(retro_win, 0, 4, (int)luna_eclipse[EN_SIGN], zo_sym, z_arr);
	mvwprintw(retro_win, 0, 8, "%2.f", luna_eclipse[EN_JUL]);
 
	zodiac_color(retro_win, 0, 13, (int)luna_eclipse[EP_SIGN], zo_sym, z_arr);
	mvwprintw(retro_win, 0, 17, "-%2.f", luna_eclipse[EP_JUL]);
	
	mvwprintw(retro_win, 1, 0, "(o)");
	zodiac_color(retro_win, 1, 4, (int)sol_eclipse[EN_SIGN], zo_sym, z_arr);
	mvwprintw(retro_win, 1, 8, "%2.f", sol_eclipse[EN_JUL]);
 
	zodiac_color(retro_win, 1, 13, (int)sol_eclipse[EP_SIGN], zo_sym, z_arr);
	mvwprintw(retro_win, 1, 17, "-%2.f", sol_eclipse[EP_JUL]);
   
    size_t i = 2, j = SE_MERCURY;
	for (; i < p_count; ++i, ++j)
	{
		char header[MAXBUF];
		snprintf(header, sizeof(header), "%-6s%6s  %4s %4s", 
		"planet", "speed", "next", "prev");
		mvwprintw(retro_win, 2, 0, "%s", header);
	
		char buff[MAXBUF];
		
		if (p_arr[j][LONG_S] > 0.0)
		{
			snprintf(buff, sizeof(buff), "%-6s%2.0f*%-2.02d'  %-4.0f %-4.0f",
			pl_sym[j], p_arr[j][DEGREE_S], (int)p_arr[j][MIN_S],
			p_arr[j][NEXT_S], p_arr[j][PREV_S]);
		}
		
		else
		{
			snprintf(buff, sizeof(buff), "%-6s-%1.0f*%-2.02d'  %-4.0f %-4.0f",
			pl_sym[j], p_arr[j][DEGREE_S], (int)p_arr[j][MIN_S],
			p_arr[j][NEXT_S], p_arr[j][PREV_S]);
		}
		
		mvwprintw(retro_win, (int)i + 1, 0, "%s", buff);
	}
}
