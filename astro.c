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

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <pwd.h>
#include <swephexp.h>
#include <ncurses.h>
#include <form.h>
#include "astro.h"

Mode mode = INSERT;

int normalize_input(WINDOW *win, int ch)
{
	// escape sequence compatibility 
	
	if (ch == 8 || ch == 127)
		return KEY_BACKSPACE;
	if (ch == 27)
	{
		nodelay(win, TRUE);
		int next = wgetch(win);
		nodelay(win, FALSE);
		
		if (next == ERR)
			return 27;
		
		if (next == '[')
		{
			int arrow = wgetch(win);
			
			switch(arrow)
			{
				case 'A':
					nodelay(win, FALSE);
					return KEY_UP;
				case 'B':
					nodelay(win, FALSE);
					return KEY_DOWN;
				case 'C':
					nodelay(win, FALSE);
					return KEY_RIGHT;
				case 'D':
					nodelay(win, FALSE);
					return KEY_LEFT;
				case 'H':
					nodelay(win, FALSE);
					return KEY_HOME;
				case 'F':
					nodelay(win, FALSE);
					return KEY_END;
				case '5':
					wgetch(win);
					nodelay(win, FALSE);
					return KEY_PPAGE;
				case '6':
					wgetch(win);
					nodelay(win, FALSE);
					return KEY_NPAGE;
				case '3':
					wgetch(win);
					nodelay(win, FALSE);
					return KEY_DC;
				case '2':
					wgetch(win);
					nodelay(win, FALSE);
					return KEY_IC;
				default:
					nodelay(win, FALSE);
					return 27;
			}
		}
		nodelay(win, FALSE);
		return 27;
	}
	return ch;
}
				
void buff_trim(FIELD *current, char *buffer)
{
	char *f_buf = field_buffer(current, 0);
	int len = 0;
	field_info(current, NULL, &len, NULL, NULL, NULL, NULL);
	
	if (len <= 0)
	{
		buffer[0] = '\0';
		return;
	}
	
	memcpy(buffer, f_buf, (size_t)len);
	
	// decrement one to be in bounds, trim
	--len;
	while(len >= 0 && buffer[len] == ' ')
		--len;
	buffer[len + 1] = '\0';
}

void field_to_member
(WINDOW *cdata_form_win, struct tm *cdata, Location *loc, 
FORM *cdata_form, FIELD *cdata_field[])
{
	FIELD *current = current_field(cdata_form);
	int index = field_index(current);
	
	char *buffer = malloc(1024);
	if (!buffer)
	{
		endwin();
		perror("field to member buffer malloc");
		ERR_EXIT;
	}
	
	buff_trim(current, buffer);
	
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
			
			buff_trim(current, buffer);
			
			loc->city = strdup(buffer);
			doupdate();
			break;
		case 1:
			cdata->tm_year = atoi(buffer);
			break;
		case 2:
			cdata->tm_mon = atoi(buffer);
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
			mvwprintw(cdata_form_win, starty, startx - 12,
			"%s", c_labels[i]);
	wrefresh(cdata_form_win);
}

void set_localtime(FIELD *cdata_field[], struct tm *cdata)
{
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
	curs_set(1);
	
	cdata_form_win = newwin(maxy - 2, maxx - 2, 0, 0);
	
	keypad(cdata_form_win, TRUE);	
	clearok(cdata_form_win, TRUE);
	
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
	cdata_field[6] = new_field(1, 30, starty, startx, 0, 0);
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
	
	set_current_field(cdata_form, cdata_field[0]);
	
	wrefresh(cdata_form_win);
	field_label(cdata_form_win, i, starty, startx);
	pos_form_cursor(cdata_form);
	
	int cdata_entry = 0;
	while(!cdata_entry && (ch = GET_INPUT(cdata_form_win)))
	{
		switch(mode)
		{	
			case NORMAL:
				switch (ch)
				{
					case 27:
						break;
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
						main_io(cdata_field, cdata, loc, mode, 'w');
						break;
					case 'e':
						main_io(cdata_field, cdata, loc, mode, 'e');
						break;
					case '\n':
						cdata_entry = 1;
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
	
	for (i = 0; i < 9; ++i)
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
		fprintf(stderr, "ERR: %s\n", cmd);
		pclose(fp);
		return;
	}
	buffer[strcspn(buffer, "\n")] = 0;
	pclose(fp);
	
	orig->tm_isdst = 
	(tzname[1] && strcmp(buffer, tzname[1]) == 0) ? 1 : 0;
}

void chart_timeset(struct tm *cdata, Location *loc, int *offset)
{
	struct tm *orig = cdata;
	check_dst(orig);
	//correct tm quirk after GNU date
	orig->tm_year -= 1900;
	orig->tm_mon -= 1;
	
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
	
	if((dhour >= 24.0))
	{
		dhour -= 23.999999;
		++orig->tm_mday;
		*offset -= 1;
	}
	if((dhour <= 0))
	{
		dhour += 23.999999;
		--orig->tm_mday;
		*offset += 1;
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
		p_deg->dsat,
		p_deg->dura,
		p_deg->dnep,
		p_deg->dplu,
		p_deg->dmnod,
		p_deg->dtnod
	};
	
	const char *pl_sym[] = {"(o)", "(()", "(-o<)",
	"(~:o)", "(o->)", "(\\+)", "(h)", "(\\*/)", "(?)",
	"(P)", NULL, "(^)"};
	
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
	int dir_x = ((int)cos(rad) != 0) ? 1 : -1;
	int dir_y = ((int)sin(rad) != 0) ? -1 : 1;
	
	bool near_horizontal = (fabs(sin(rad)) < 0.5);

	for (int j = 0; j < i; j++)
	{
		if (fabs(planet - p_arr[j]) <= 8)	
		{
			if (near_horizontal)
			{
				offsetx += 7;
				offsety += 2;
			}
			else
			{
				offsety -= 3;
				offsetx += 4;
			}
		}
	}
	
	if (!near_horizontal)
		offsety = dir_y * offsety;
	
	offsetx = dir_x * offsetx;
	
	double decimal  = (((planet - (int)planet) * 60) / 100);
	
	char buffer[56];
	snprintf(buffer, sizeof(buffer), "%.2f", ((int)planet % 30) +
	decimal);
	
	if (i != 10)
	{
		mvwaddstr(main_win, (y + offsety) - 1, x + offsetx + 1, buffer);
	
		mvwaddstr(main_win, y + offsety, x + offsetx, pl_sym[i]);
	}
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
			int line_y = center_y + (int)(r * sin(rad) * 0.5);
			
			if (line_x >= 0 && line_x < maxx
			&& line_y >= 0 && line_y < maxy)
				mvwaddch(main_win, line_y, line_x, '`');
		}
	}
	
	mvwaddstr(main_win, y, x, ascmc_sym[i]);
	
	double decimal = (((angle - (int)angle) * 60) / 100);
	
	char buffer[56];
	snprintf(buffer, sizeof(buffer), "%.2f", ((int)angle % 30) + 
	decimal);
	
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

void draw_chart(WINDOW *main_win, int maxy, int maxx,
struct tm *cdata, Location *loc, P_deg *p_deg)
{
	int iret, iflag, ipl, i;
	double xx[6];
	char serr[AS_MAXCH];
	char spname[AS_MAXCH];
	double cusps[13], ascmc[10]; //houses, asc, mc
	int ihsy = 'W'; // house system
	
	int offset = 0;
	
	double *p_deg_members[] = {
	&p_deg->dsun, &p_deg->dmoon,
	&p_deg->dmerc, &p_deg->dven,
	&p_deg->dmars, &p_deg->djup,
	&p_deg->dsat, &p_deg->dura,
	&p_deg->dnep, &p_deg->dplu,
	&p_deg->dmnod, &p_deg->dtnod};
	
	chart_timeset(cdata, loc, &offset); // goes before swe_julday
	reset_struct(cdata); // ----
	
	double jul_day_UT = swe_julday(cdata->tm_year, cdata->tm_mon, 
	cdata->tm_mday, loc->dhour, SE_GREG_CAL);
	
	// corrects loc->dhour offset from chart_timeset()
	cdata->tm_mday += offset;

	wclear(main_win);	
	
	iflag = SEFLG_SWIEPH | SEFLG_SPEED;
	for (ipl = SE_SUN, i = 0; ipl <= SE_TRUE_NODE; ipl++, i++)
	{
		swe_get_planet_name(ipl, spname);
		spname[17] = '\0';
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

	for (i = 0; i < 12; ++i)
	{
		planet_pos(main_win, i, maxy, maxx,
		radius - 6, *p_deg_members[i], cusps[1], p_deg);
	}
	
	for (i = 0; i < 2; ++i)
	{
		ascmc_pos(main_win, i, maxy, maxx,
		(radius / 2) + 4 , ascmc[i], cusps[1]);
	}
		
	wrefresh(main_win);
}

void display_data(WINDOW *main_win, int maxx, 
struct tm *cdata, Location *loc)
{	
	int starty = 1;
	int startx = maxx - 22;
	
	mvwprintw(main_win, starty, startx, "%s", loc->city);
	
	starty += 1;
	mvwprintw(main_win, starty, startx, "%d", cdata->tm_year);
	
	starty += 1;
	mvwprintw(main_win, starty, startx, "%d/%d", cdata->tm_mon, cdata->tm_mday);
	
	starty += 1;
	mvwprintw(main_win, starty, startx, "%d:%d", cdata->tm_hour, cdata->tm_min);
	
	starty += 1;
	mvwprintw(main_win, starty, startx, "lat.%.3f", loc->dlat);
	
	starty += 1;
	mvwprintw(main_win, starty, startx, "lon.%.3f", loc->dlon);
	
	wrefresh(main_win);
}

int months(int month, int year)
{
	int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	if (month == 2)
		if ((year % 4 == 0 && year % 100 != 0) || 
		(year % 400 == 0))
			return 29;
	return days[month];
}

void animate_chart(WINDOW *main_win, int maxy, int maxx,
struct tm *cdata, Location *loc, P_deg *p_deg)
{
	int starty = 8;
	int startx = maxx - 22;
	
	wrefresh(main_win);
	
	size_t i = 0;
	int ch = 0;
	int anim_done = 0;
	while(!anim_done && (ch = GET_INPUT(main_win)))
	{
		switch(ch)
		{
			case 'h':
				if (i != 0)
					--i;
				break;
			case 'l':
				if (i != 5)
					++i;
				break;
			case 'j':
				switch(i)
				{
					case 0:
						if ((++cdata->tm_min) > 59)
						{
							++cdata->tm_hour;
							cdata->tm_min = 0;
							if ((cdata->tm_hour) > 23)
							{
								cdata->tm_hour = 0;
								++cdata->tm_mday;
								if (cdata->tm_mday > months(
								cdata->tm_mon, cdata->tm_year))
								{
									cdata->tm_mday = 1;
									++cdata->tm_mon;
									if (cdata->tm_mon > 12)
									{
										cdata->tm_mon = 1;
										++cdata->tm_year;
									}
								}
							}
						}
						draw_chart(main_win, maxy,
						maxx, cdata, loc, p_deg);
						display_data(main_win, maxx, cdata, loc);
						break;
					case 1:
						if ((++cdata->tm_hour) > 23)
						{
							cdata->tm_hour = 0;
							++cdata->tm_mday;
							if (cdata->tm_mday > months(
							cdata->tm_mon, cdata->tm_year))
							{
								cdata->tm_mday = 1;
								++cdata->tm_mon;
								if (cdata->tm_mon > 12)
								{
									cdata->tm_mon = 1;
									++cdata->tm_year;
								}
							}
						}
						draw_chart(main_win, maxy,
						maxx, cdata, loc, p_deg);
						display_data(main_win, maxx, cdata, loc);
						break;
					case 2:
						if ((++cdata->tm_mday) > months(
						cdata->tm_mon, cdata->tm_year))
						{
							cdata->tm_mday = 1;
							++cdata->tm_mon;
							if (cdata->tm_mon > 12)
							{
								cdata->tm_mon = 1;
								++cdata->tm_year;
							}
						}
						draw_chart(main_win, maxy,
						maxx, cdata, loc, p_deg);
						display_data(main_win, maxx, cdata, loc);
						break;
					case 3:
						if ((++cdata->tm_mon) > 12)
						{
							cdata->tm_mon = 1;
							++cdata->tm_year;
						}
						int max_day = months(
						cdata->tm_mon, cdata->tm_year);
						if (cdata->tm_mday > max_day)
							cdata->tm_mday = max_day;
							
						draw_chart(main_win, maxy,
						maxx, cdata, loc, p_deg);
						display_data(main_win, maxx, cdata, loc);
						break;
					case 4:
						if ((++cdata->tm_year) > 16799)
							cdata->tm_year = -12998;
						draw_chart(main_win, maxy,
						maxx, cdata, loc, p_deg);
						display_data(main_win, maxx, cdata, loc);
						break;
				}
				break;
			case 'k':
				switch(i)
				{
					case 0:
						if ((--cdata->tm_min) < 0)
						{
							--cdata->tm_hour;
							cdata->tm_min = 59;
							if ((cdata->tm_hour) < 0)
							{
								cdata->tm_hour = 23;
								--cdata->tm_mday;
								if (cdata->tm_mday < 1)
								{
									--cdata->tm_mon;
									if (cdata->tm_mon < 1)
									{
										cdata->tm_mon = 12;
										--cdata->tm_year;
									}
									cdata->tm_mday = months(
									cdata->tm_mon, cdata->tm_year);
								}
							}
						}
						draw_chart(main_win, maxy,
						maxx, cdata, loc, p_deg);
						display_data(main_win, maxx, cdata, loc);
						break;
					case 1:
						if ((--cdata->tm_hour) < 0)
						{
							cdata->tm_hour = 23;
							--cdata->tm_mday;
							if (cdata->tm_mday < 1)
							{
								--cdata->tm_mon;
								if (cdata->tm_mon < 1)
								{
									cdata->tm_mon = 12;
									--cdata->tm_year;
								}
								cdata->tm_mday = months(
								cdata->tm_mon, cdata->tm_year);
							}
						}
						draw_chart(main_win, maxy,
						maxx, cdata, loc, p_deg);
						display_data(main_win, maxx, cdata, loc);
						break;
					case 2:
						if ((--cdata->tm_mday) < 1)
						{
							--cdata->tm_mon;
							if (cdata->tm_mon < 1)
							{
								cdata->tm_mon = 12;
								--cdata->tm_year;
							}
							cdata->tm_mday = months(
							cdata->tm_mon, cdata->tm_year);
						}
						draw_chart(main_win, maxy,
						maxx, cdata, loc, p_deg);
						display_data(main_win, maxx, cdata, loc);
						break;
					case 3:
						if (--cdata->tm_mon < 1)
						{
							cdata->tm_mon = 12;
							--cdata->tm_year;
						}
						int max_day = months(
						cdata->tm_mon, cdata->tm_year);
						if (cdata->tm_mday > max_day)
							cdata->tm_mday = max_day;
						draw_chart(main_win, maxy,
						maxx, cdata, loc, p_deg);
						display_data(main_win, maxx, cdata, loc);
						break;
					case 4:
						if ((--cdata->tm_year) < -12998)
							cdata->tm_year = 16799;
						draw_chart(main_win, maxy,
						maxx, cdata, loc, p_deg);
						display_data(main_win, maxx, cdata, loc);
						break;
				}
				break;
			case '\n':
				anim_done = 1;
				break;
		}
		
		switch(i)
		{
			case 0:
				wmove(main_win, starty, startx);
				wclrtoeol(main_win);
				mvwprintw(main_win, starty, startx, "(min)");
				wrefresh(main_win);
				break;
			case 1:
				wmove(main_win, starty, startx);
				wclrtoeol(main_win);
				mvwprintw(main_win, starty, startx, "(hour)");
				wrefresh(main_win);
				break;
			case 2:
				wmove(main_win, starty, startx);
				wclrtoeol(main_win);
				mvwprintw(main_win, starty, startx, "(day)");
				wrefresh(main_win);
				break;
			case 3:
				wmove(main_win, starty, startx);
				wclrtoeol(main_win);
				mvwprintw(main_win, starty, startx, "(mon)");
				wrefresh(main_win);
				break;
			case 4:
				wmove(main_win, starty, startx);
				wclrtoeol(main_win);
				mvwprintw(main_win, starty, startx, "(year)");
				wrefresh(main_win);
				break;
		}
	}
	wmove(main_win, starty, startx);
	wclrtoeol(main_win);
	wrefresh(main_win);
}

int main()
{
	WINDOW *main_win;

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

	int maxy, maxx;
	
	initscr();
	getmaxyx(stdscr, maxy, maxx);
	
	swe_set_ephe_path("/home/plum/Builds/swisseph/ephe");

	main_win = newwin(maxy, maxx, 0, 0);
	keypad(main_win, TRUE);
	keypad(stdscr, TRUE);
	
	int main_done = 0;
	while (!main_done)
	{
		ichart_data(cdata, loc);
		draw_chart(main_win, maxy, maxx, cdata, loc, p_deg);
		display_data(main_win, maxx, cdata, loc);
			
		int chart_done = 0, ch = 0;
		while(!chart_done && !main_done &&
		(ch = GET_INPUT(main_win)))
		{
			switch(ch)
			{
				case '\n':
					animate_chart(main_win, maxy, maxx,
					cdata, loc, p_deg);
					break;
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
	free(loc->city);
	free(loc);
	free(p_deg);
	return 0;
}

