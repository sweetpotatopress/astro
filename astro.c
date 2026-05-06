/* Copyright (C) 2026 yam lynn
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by the 
Free Software Foundation, either version 3 of the License, or (at your option)
any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTIBILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. if not, see <https://www.gnu.org/licenses/> */

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <pwd.h>
#include <swephexp.h>
#include <ncurses.h>
#include <form.h>
#include <panel.h>
#include "astro.h"
#include "search.c"
#include "io.c"

void field_to_member
(WINDOW *cdata_form_win, struct tm *cdata, Location *loc, 
FORM *cdata_form, FIELD *cdata_field[])
{
	FIELD *current = current_field(cdata_form);
	char *f_buf = field_buffer(current, 0);
	int index = field_index(current);
	
	// get field buffer length, copy to buffer
	int len = 0;
	field_info(current, NULL, &len, NULL, NULL, NULL, NULL);
	
	char *buffer = malloc((size_t)len + 1);
	if (!buffer)
	{
		endwin();
		perror("field to member buffer malloc");
		ERR_EXIT;
	}
	
	memcpy(buffer, f_buf, (size_t)len);
	
	// decrement one to be in bounds, trim
	--len;
	while(len >= 0 && buffer[len] == ' ')
	{
		--len;
	}
	if (len >= 0)
		buffer[len + 1] = '\0';
	
	switch(index)
	{
		case 0:
			main_search(cdata_field, buffer);
			form_driver(cdata_form, REQ_VALIDATION);
			
			//redraws field underline
			unpost_form(cdata_form);
			touchwin(cdata_form_win);
			post_form(cdata_form);
			wrefresh(cdata_form_win);
			
			loc->city = buffer;
			doupdate();
			break;
		case 1:
			cdata->tm_year = atoi(buffer);
			break;
		case 2:
			cdata->tm_mon = atoi(buffer) - 1;
			break;
		case 3: 
			cdata->tm_mday = atoi(buffer);
			break;
		case 4:
			cdata->tm_hour = atoi(buffer);
			break;
		case 5:
			cdata->tm_min = atoi(buffer);
			break;
		case 6:
			if (setenv("TZ", buffer, 1) != 0)
			{
				endwin();
				perror("TZ setenv");
				ERR_EXIT;
			}
			tzset();
			break;
		case 7:
			loc->dlat = atof(buffer);
			break;
		case 8:
			loc->dlon = atof(buffer);
			break;
	}
	free(buffer);
}

void field_label(WINDOW *cdata_form_win, size_t i, int starty, int startx)
{
	const char *c_labels[] = {
		"city search:",
		"year:",
		"month:",
		"day:",
		"hour:",
		"minute:",
		"timezone:",
		"lat.",
		"long.",
		NULL
	};
	
	for (i = 0, starty = 4; i < 9; ++i, starty+= 2)
			mvwprintw(cdata_form_win, starty, startx - 12, "%s", c_labels[i]);
	wrefresh(cdata_form_win);
}

void set_localtime(FIELD *cdata_field[], struct tm *cdata)
{
	cdata = malloc(sizeof(struct tm));
	char buff[128] = {0};
	ssize_t len = readlink("/etc/localtime", buff, sizeof(buff) - 1);
	if (len != -1)
	{
		buff[len] = 0;
		
		char *tz = strstr(buff, "zoneinfo/");
		if (tz)
			memmove(buff, tz + 9, strlen(tz + 9) + 1);
	}
	setenv("TZ", buff, 1);
	tzset();
	
	time_t now = time(NULL);
	localtime_r(&now, cdata);
	set_field_buffer(cdata_field[6], 0, buff);
	
	memset(buff, 0, sizeof(buff));
	snprintf(buff, sizeof(buff), "%d", cdata->tm_year+1900);
	set_field_buffer(cdata_field[1], 0, buff);
	
	memset(buff, 0, sizeof(buff));
	snprintf(buff, sizeof(buff), "%d", cdata->tm_mon + 1);
	set_field_buffer(cdata_field[2], 0, buff);
	
	memset(buff, 0, sizeof(buff));
	snprintf(buff, sizeof(buff), "%d", cdata->tm_mday);
	set_field_buffer(cdata_field[3], 0, buff);
	
	memset(buff, 0, sizeof(buff));
	snprintf(buff, sizeof(buff), "%d", cdata->tm_hour);
	set_field_buffer(cdata_field[4], 0, buff);
	
	memset(buff, 0, sizeof(buff));
	snprintf(buff, sizeof(buff), "%d", cdata->tm_min);
	set_field_buffer(cdata_field[5], 0, buff);
	
}

void validate_fields(WINDOW *cdata_form_win, FIELD *cdata_field[],
FORM *cdata_form, struct tm *cdata, Location *loc)
{
	size_t i = 0;
	//validates every field, in case user didnt hit enter
	for (i = 1; i < 9; i++)
	{
		set_current_field(cdata_form, cdata_field[i]);
		form_driver(cdata_form, REQ_VALIDATION);
		field_to_member(cdata_form_win, cdata, loc, 
		cdata_form, cdata_field);
	}
}
	
void ichart_data(struct tm *cdata, Location *loc)
{
	WINDOW *cdata_form_win;
	FIELD *cdata_field[10];
	FORM *cdata_form;
	int ch;
	int starty, startx;
	size_t i = 0;
	
	int maxy, maxx;
	getmaxyx(stdscr, maxy, maxx);

	cbreak();
	noecho();
	
	cdata_form_win = newwin(maxy - 2, maxx - 2, 0, 0);
	keypad(cdata_form_win, TRUE);
	clearok(cdata_form_win, TRUE);
	wclear(cdata_form_win);
	wrefresh(cdata_form_win);
	
	starty = 4;
	startx = 18;
	
	// city search
	cdata_field[0] = new_field(1, 25, starty, startx, 0, 0);
	set_field_back(cdata_field[0], A_UNDERLINE);
	field_opts_off(cdata_field[0], O_STATIC);
	field_opts_off(cdata_field[0], O_AUTOSKIP);
	starty += 2;
	// year
	cdata_field[1] = new_field(1, 6, starty, startx, 0, 0);
	set_field_back(cdata_field[1], A_UNDERLINE);
	field_opts_off(cdata_field[1], O_AUTOSKIP);
	starty += 2;
	// month
	cdata_field[2] = new_field(1, 3, starty, startx, 0, 0);
	set_field_back(cdata_field[2], A_UNDERLINE);
	field_opts_off(cdata_field[2], O_AUTOSKIP);
	starty += 2;
	// day
	cdata_field[3] = new_field(1, 3, starty, startx, 0, 0);
	set_field_back(cdata_field[3], A_UNDERLINE);
	field_opts_off(cdata_field[3], O_AUTOSKIP);
	starty += 2;
	// hour
	cdata_field[4] = new_field(1, 3, starty, startx, 0, 0);
	set_field_back(cdata_field[4], A_UNDERLINE);
	field_opts_off(cdata_field[4], O_AUTOSKIP);
	starty+= 2;
	// minute
	cdata_field[5] = new_field(1, 3, starty, startx, 0, 0);
	set_field_back(cdata_field[5], A_UNDERLINE);
	field_opts_off(cdata_field[5], O_AUTOSKIP);
	starty+= 2;
	// timezone
	cdata_field[6] = new_field(1, 25, starty, startx, 0, 0);
	set_field_back(cdata_field[6], A_UNDERLINE);
	field_opts_off(cdata_field[6], O_STATIC);
	field_opts_off(cdata_field[6], O_AUTOSKIP);
	starty += 2;
	// lat. 
	cdata_field[7] = new_field(1, 11, starty, startx, 0, 0);
	set_field_back(cdata_field[7], A_UNDERLINE);
	field_opts_off(cdata_field[7], O_AUTOSKIP);
	starty+= 2;
	// long.
	cdata_field[8] = new_field(1, 11, starty, startx, 0, 0);
	set_field_back(cdata_field[8], A_UNDERLINE);
	field_opts_off(cdata_field[8], O_AUTOSKIP);
	
	cdata_field[9] = NULL;

	cdata_form = new_form(cdata_field);
	set_form_win(cdata_form, cdata_form_win);
	set_form_sub(cdata_form,
	derwin(cdata_form_win, maxy - 6, maxx - 6, 0, 0));
	
	touchwin(cdata_form_win);
	post_form(cdata_form);
	wrefresh(cdata_form_win);
	
	set_current_field(cdata_form, cdata_field[0]);
	
	wrefresh(cdata_form_win);
	field_label(cdata_form_win, i, starty, startx);
	pos_form_cursor(cdata_form);
	
	int done = 0;
	while(!done && (ch = wgetch(cdata_form_win)))
	{
		switch(mode)
		{	
			case NORMAL:
				switch (ch)
				{
					case 'i':
						mode = INSERT;
						break;
					case 'j': case KEY_DOWN:
						form_driver(cdata_form, REQ_NEXT_FIELD);
						form_driver(cdata_form, REQ_END_LINE);
						break;
					case 'k': case KEY_UP:
						form_driver(cdata_form, REQ_PREV_FIELD);
						form_driver(cdata_form, REQ_END_LINE);
						break;
					case 'h': case KEY_LEFT:
						form_driver(cdata_form, REQ_LEFT_CHAR);
						break;
					case 'l': case KEY_RIGHT:
						form_driver(cdata_form, REQ_RIGHT_CHAR);
						break;
					case 9: // tab
						set_localtime(cdata_field, cdata);
						break;
					case 'w':
						validate_fields(cdata_form_win, cdata_field,
						cdata_form, cdata, loc);
						main_io(cdata_field, cdata, loc, 'w');
						break;
					case 'e':
						main_io(cdata_field, cdata, loc, 'e');
						break;
					case '\n':
						done = 1;
						break;
				}
				break;
			case INSERT:
				switch (ch)
				{
					 case '\n':
						form_driver(cdata_form, REQ_VALIDATION);
						field_to_member(cdata_form_win, cdata, loc,
						cdata_form, cdata_field);
						form_driver(cdata_form, REQ_NEXT_FIELD);
						
						field_label(cdata_form_win, i, starty, startx);
						
						form_driver(cdata_form, REQ_END_LINE);
						break;
					case KEY_DOWN:
						form_driver(cdata_form, REQ_NEXT_FIELD);
						form_driver(cdata_form, REQ_END_LINE);
						break;
					case KEY_UP:
						form_driver(cdata_form, REQ_PREV_FIELD);
						form_driver(cdata_form, REQ_END_LINE);
						break;
					case KEY_LEFT:
						form_driver(cdata_form, REQ_LEFT_CHAR);
						break;
					case KEY_RIGHT:
						form_driver(cdata_form, REQ_RIGHT_CHAR);
						break;
					case KEY_BACKSPACE:
						form_driver(cdata_form, REQ_DEL_PREV);
						break;
					case 27: // esc
						mode = NORMAL;
						break;
					default:
						form_driver(cdata_form, ch);
						break;
					}
					break;
		}
		wrefresh(cdata_form_win);
	}
	
	validate_fields(cdata_form_win, cdata_field,
	cdata_form, cdata, loc);

	unpost_form(cdata_form);
	wclear(cdata_form_win);
	touchwin(cdata_form_win);
	wrefresh(cdata_form_win);
	free_form(cdata_form);
	
	for (i = 0; i < 7; ++i)
	{
		free_field(cdata_field[i]);
	}
	delwin(cdata_form_win);
}
void check_dst(struct tm *orig)
{
	char cmd[256] = {0};
	char buffer[256] = {0};
	char *tz_name = getenv("TZ");
	FILE *fp;
	
	//calls GNU coreutil date
	snprintf(cmd, sizeof(cmd), "TZ=%s date -d '%d-%d-%d %d:%d' '+%%Z'", 
	tz_name, orig->tm_year, orig->tm_mon, orig->tm_mday,
	orig->tm_hour, orig->tm_min);
	
	fp = popen(cmd, "r");
	if (!fp)
	{
		endwin();
		perror("ERR: date file pointer");
		ERR_EXIT;
	}
	
	if (fgets(buffer, sizeof(buffer), fp) == NULL)
	{
		endwin();
		perror("ERR:dst fgets");
		pclose(fp);
		return;
	}
	buffer[strcspn(buffer, "\n")] = 0;
	pclose(fp);
	
	orig->tm_isdst = 
	(tzname[1] && strcmp(buffer, tzname[1]) == 0) ? 1 : 0;
}

void chart_timeset(struct tm *cdata, Location *loc)
{
	struct tm *orig = cdata;
	check_dst(orig);
	//correct tm quirk after GNU date
	orig->tm_year -= 1900;
	
	//copy correct isdst and hour before mktime
	//mktime "corrects" it to system defaults, which can be wrong
	int isdst = orig->tm_isdst;
	int tm_hour = orig->tm_hour;
	
	time_t tret = mktime(orig);
	localtime_r(&tret, orig);
	
	orig->tm_isdst = isdst;
	orig->tm_hour = tm_hour;

	long utc_sec = orig->tm_gmtoff;
	
	//get utc offset in seconds, reverse, and display in hours
	double utc_offset = (double)-utc_sec / 3600;
	
	//convert inputted minutes to decimal
	double min = (double)orig->tm_min / 60;
	
	//add inputted hour, utc offset, and minutes to decimal
	//swe_julday uses 24 hour UTC.
	double dhour = (double)(orig->tm_hour + utc_offset) + min;
	
	if(dhour > 23.999999)
	{
		dhour -= 23.999999;
		++orig->tm_mday;
	}
	if(dhour < 0)
	{
		dhour += 23.999999;
		--orig->tm_mday;
	}
	
	loc->dhour = dhour; 
	*cdata = *orig;
}

void reset_struct(struct tm *cdata)
{
	cdata->tm_mon += 1;
	cdata->tm_year += 1900;
}

void draw_circle(WINDOW *main_win,
int maxy, int maxx, int radius, chtype ch)
{
	int center_x = maxx / 2;
	int center_y = maxy / 2;
	
	int x = 0;
	int y = radius;
	int d = 3 -2 * radius;
	
	while (x <= y)
	{
		mvwaddch(main_win, center_y + y / 2, center_x + x, ch);
		mvwaddch(main_win, center_y + y / 2, center_x - x, ch);
		mvwaddch(main_win, center_y - y / 2, center_x + x, ch);
		mvwaddch(main_win, center_y - y / 2, center_x - x, ch);
		
		mvwaddch(main_win, center_y + x / 2, center_x + y, ch);
		mvwaddch(main_win, center_y + x / 2, center_x - y, ch);
		mvwaddch(main_win, center_y - x / 2, center_x + y, ch);
		mvwaddch(main_win, center_y - x / 2, center_x - y, ch);
	
		if (d < 0)
			d = d + 4 * x + 6;
		else
		{
			d = d + 4 * (x - y) + 10;
			y--;
		}
		x++;
	}
}

void planet_pos(WINDOW *main_win, int i, int maxy, int maxx,
int radius, double planet, double asc, P_deg *p_deg)
{
	double p_arr[] = {
		p_deg->dsun,
		p_deg->dmoon,
		p_deg->dmerc,
		p_deg->dven,
		p_deg->dmars,
		p_deg->djup,
		p_deg->dsat
	};
	
	const char *pl_sym[] = {"(o)", "(()", "(-o<)",
	"(~:o)", "(o->)", "(\\+)", "(h)"};
	
	int center_x = (maxx / 2);
	int center_y = (maxy / 2);
	
	double rad = (planet - asc) * M_PI / 180.0;
	
	int x = center_x - (int)(radius * cos(rad));
	int y = center_y + (int)(radius * sin(rad) * 0.5);
	
	int offsety = 0;
	int offsetx = 0;
	/* 	cos	1	0	-1	0
			0	90	180	270
		sin	0	1	0	-1
	*/
	int dir_y = (sin(rad) < 0) ? 1 : -1;
	int dir_x = ((int)cos(rad) != 0) ? 1 : -1;
	
	for (int j = 0; j < i; j++)
	{
		if (fabs(planet - p_arr[j]) <= 8)	
		{
			offsety += 3;
			offsetx -= 2;
		}
	}
	for (int j = 0; j < i; j++)
	{
		if (fabs(planet - p_arr[j]) <= 8)	
		{
			offsetx += 5;
			offsety -= 1;
		}
	}
	
	offsety = dir_y * offsety;
	offsetx = dir_x * offsetx;
	
	//print planets degree
	char buffer[56];
	snprintf(buffer, sizeof(buffer), "%d", (int)planet % 30);
	mvwaddstr(main_win, (y + offsety) - 1, x + offsetx, buffer);
	
	mvwaddstr(main_win, y + offsety, x + offsetx, pl_sym[i]);
}

void ascmc_pos(WINDOW *main_win, int i, int maxy, int maxx,
int radius, double angle, double asc)
{
	const char *ascmc_sym[] = {"as", "mc"};
	int center_x = (maxx / 2);
	int center_y = (maxy / 2);
	
	double rad = (angle - asc) * M_PI / 180.0;
	
	int x = center_x - (int)(radius * cos(rad));
	int y = center_y + (int)(radius * sin(rad) * 0.5);
	
	if (i == 0) // draw asc line
	{
		for (int r = 0; r <= radius; r++)
		{
			int line_x = center_x - (int)(r * cos(rad));
			
			if (line_x >= 0 && line_x < maxx)
				mvwaddch(main_win, y, line_x, '-');
		}
	}
	
	mvwaddstr(main_win, y, x, ascmc_sym[i]);
	char buffer[56];
	snprintf(buffer, sizeof(buffer), "%d", (int)angle % 30);
	mvwaddstr(main_win, y - 1, x, buffer);

}

void zo_pos(WINDOW *main_win, int i, int maxy, int maxx,
int radius, double angle, double asc)
{
		
	const char *zo_sym[] = {NULL, "ari", "tau", "gem", "can",
	"leo", "vir", "lib", "sco", "sag",
	"cap", "aqu", "pis"};
	
	
	int center_x = (maxx / 2);
	int center_y = (maxy / 2);
	
	int sign = (((int)asc / 30) * 30) + 15;
	
	double rad = (angle - sign) * M_PI / 180.0;
	
	int x = center_x - (int)(radius * cos(rad));
	int y = center_y + (int)(radius * sin(rad) * 0.5);
	
	mvwaddstr(main_win, y, x, zo_sym[i]);
}

void draw_house(WINDOW *main_win, int maxy, int maxx, 
int radius, double angle, chtype ch)
{
	double rad = angle * M_PI / 180.0;
	
	int center_x = maxx / 2;
	int center_y = maxy / 2;
	
	int edge_x = center_x - (int)(radius * cos(rad));
	int edge_y = center_y + (int)(radius * sin(rad) * 0.5);
	
	int half_x = center_x - (int)((radius / 2) * cos(rad));
	int half_y = center_y + (int)((radius / 2)  * sin(rad) * 0.5);
	
	int dx = edge_x - half_x;
	int dy = edge_y - half_y;
	
	int distance = (int)sqrt(dx * dx + dy * dy);
	
	for(int i = 0; i <= distance; i++)
	{
		int x = half_x + (dx * i) / distance;
		int y = half_y + (dy * i) / distance;
		mvwaddch(main_win, y, x, ch);
	}
}

int main()
{
	WINDOW *main_win;
	int iret, iflag, ipl, i, c;
	double xx[6];
	char serr[AS_MAXCH];
	char spname[AS_MAXCH];
	double cusps[13], ascmc[10]; //houses, asc, mc
	int ihsy = 'W'; // house system
	
	struct tm *cdata = calloc(1, sizeof(struct tm));
	if (!cdata)
	{
		endwin();
		perror("Cdata calloc");
		ERR_EXIT;
	}
	
	Location *loc = calloc(1, sizeof(Location));
	if (!loc)
	{
		endwin();
		perror("main Location calloc");
		ERR_EXIT;
	}
	
	P_deg *p_deg = calloc(1, sizeof(P_deg));
	if (!p_deg)
	{
		endwin();
		perror("P_deg calloc");
		ERR_EXIT;
	}
	
	//to fill each member of P_deg with its planets degree in later loops
	double *p_deg_members[] = {
	&p_deg->dsun, &p_deg->dmoon,
	&p_deg->dmerc, &p_deg->dven,
	&p_deg->dmars, &p_deg->djup,
	&p_deg->dsat};
	
	int maxy, maxx;
	
	initscr();
	getmaxyx(stdscr, maxy, maxx);
	cbreak();
	swe_set_ephe_path("/home/plum/Builds/swisseph/ephe");

	main_win = newwin(maxy, maxx, 0, 0);
	keypad(main_win, TRUE);
	wrefresh(main_win);
	
	int main_done = 0;
	while (!main_done)
	{
		curs_set(1);
		ichart_data(cdata, loc); // ----
		chart_timeset(cdata, loc); // goes before swe_julday
		reset_struct(cdata); // ----
		
		double jul_day_UT = swe_julday(cdata->tm_year, cdata->tm_mon, 
		cdata->tm_mday, loc->dhour, SE_GREG_CAL);

		refresh();
		
		wrefresh(main_win);
		
		iflag = SEFLG_SWIEPH | SEFLG_SPEED;
		for (ipl = SE_SUN, i = 0; ipl <= SE_SATURN; ipl++, i++)
		{
			swe_get_planet_name(ipl, spname);
			spname[7] = '\0';
			iret = swe_calc_ut(jul_day_UT, ipl, iflag, xx, serr);
			if (iret < 0) 
			{
				fprintf(stderr, "%s", serr);
				ERR_EXIT;
				exit(EXIT_FAILURE);
			}
			*p_deg_members[i] = xx[0];
		}
		
		iret = swe_houses_ex(jul_day_UT, 0, loc->dlat, loc->dlon,
		ihsy, cusps, ascmc);
		if (iret < 0)
		{
			fprintf(stderr, "%s", serr);
			ERR_EXIT;
			exit(EXIT_FAILURE);
		}
		
		curs_set(0);
		int radius = ((maxx / 2 < maxy) ? maxx / 2 : maxy) - 5;
		
		// zodiac circle
		draw_circle(main_win, maxy, maxx, radius + 4, '`');
		// outer circle
		draw_circle(main_win, maxy, maxx, radius, '.');
		// inner circle
		draw_circle(main_win, maxy, maxx, (radius / 2) - 1, '.');
		
		for (i = 0; i < 13; ++i)
		{
			draw_house(main_win, maxy, maxx, radius + 4,
			cusps[i], '`');
		}
		
		for (i = 1; i < 13; ++i)
		{
			int asc_sign = (int)(ascmc[0] / 30);
			
			int sign_display = ((i + asc_sign - 1) % 12);
			if (sign_display == 0)
				sign_display = 12;
	
			zo_pos(main_win, sign_display, maxy, maxx,
			radius + 3, cusps[i], ascmc[0]);
		}
	
		for (i = 0; i < 7; ++i)
		{
			planet_pos(main_win, i, maxy, maxx,
			radius - 4, *p_deg_members[i], cusps[1], p_deg);
		}
		
		for (i = 0; i < 2; ++i)
		{
			ascmc_pos(main_win, i, maxy, maxx,
			(radius / 2) + 4 , ascmc[i], cusps[1]);
		}
			
		wrefresh(main_win);
		
		int chart_done = 0;
		while(!chart_done && !main_done)
		{
			c = wgetch(main_win);
			switch(c)
			{
				case 'q':
					main_done = 1;
					chart_done = 1;
					break;
				case 'i':
					wclear(main_win);
					wrefresh(main_win);
					chart_done = 1;
					mode = INSERT;
					break;
				default:
					break;
			}
		}
	}
	delwin(main_win);
	endwin();
	swe_close();
	free(cdata);
	free(loc);
	free(p_deg);
	return 0;
}

