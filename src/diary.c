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

#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <astro.h>
#include <init.h>
#include <ui.h>
#include <swephexp.h>
#include <chronos.h>
#include <errno.h>

#define PLINE 2560
#define OLINE (PLINE-1)
#define PCOL 48
#define CCOL (PCOL-3)
#define NCOL (PCOL-2)
#define OCOL (PCOL-1)

#define PMAX (PLINE * PCOL)

static void d_timezone(char *timezone)
{
	if (setenv("TZ", timezone, 1) != 0)
		ERR_EXIT("ERR: setenv diary.c");
	tzset();
}

static void diary_path(char *full_path, int year, int mon, int mday)
{
	char filename[12] = {0};
	int n = snprintf(filename, 12, "%02d.%02d.md", mon, mday);
	if (n > 12)
		ERR_EXIT("ERR: diary filename too long");
	
	char path[MAXPATH] = {0};
	path_check(path, "diary");
	
	char year_path[MAXPATH] = {0};
	n = snprintf(year_path, MAXPATH, "%s/%d", path, year);
	if (n > MAXPATH)
		ERR_EXIT("ERR: diary.c year_path too long");
	
	if (access(year_path, R_OK) == -1)
		if (mkdir(year_path, 0755) == -1)
			ERR_EXIT("ERR: diary mkdir");
		
	n = snprintf(full_path, MAXPATH, "%s/%s", year_path, filename);
	if (n > MAXPATH)
		ERR_EXIT("ERR: diary.c full_path too long");
}

static void save_diary(struct cdata *cdata, struct ui *ui, char *buf, int size)
{
	int i = 0;
	for (; buf[i] != '\0' && isspace((unsigned char)buf[i]); ++i)
		;
	if (i == size || buf[i] == '\0')
		return;
		
	struct cdata time = {0};
	d_timezone(ui->conf_timezone);
	set_localtime(&time);
	d_timezone(cdata->timezone);
	
	char *tod = (time.hour > 11) ? "PM" : "AM";
	int th = time_format(time.hour);
	
	char full_path[MAXPATH] = {0};
	diary_path(full_path, time.year, time.mon, time.mday);

	char header[MAXBUF] = {0};
	if (access(full_path, R_OK) == -1)
		snprintf(header, MAXBUF, "%s,%s,%s,%d,%d,%d,%d,%d,%d,%s,%f,%f\n",
		cdata->city, cdata->state, cdata->country,
		time.year, time.mon, time.mday,
		time.hour, time.min, time.sec, 
		ui->conf_timezone, ui->conf_dlon, ui->conf_dlat);
	else
		snprintf(header, MAXBUF, "%02d:%02d:%02d%s", th, time.min, time.sec, tod);
	char *footer = "\n- - - - - - -";
	
	FILE *fp = fopen(full_path, "a");
	if (!fp)
		ERR_EXIT("ERR: diary fopen");

	fprintf(fp, "%s\n%s%s\n", header, buf, footer);
		
	fclose(fp);
}

static void end_line(char buf[PLINE][PCOL], int len[PLINE], int y)
{
	buf[y][len[y]] = '\n';
	buf[y][len[y]+1] = '\0';
}

static void up_line(int *y)
{
	if (*y > 0)
		(*y)--;
}

static void down_line(int *y)
{
	if (*y < OLINE)
		(*y)++;
}

static void down_pad(struct ui *ui)
{
	if (ui->pad_y < OLINE - ui->scr_h - 1)
	{
		ui->pad_y += ui->inc;
		ui->tl += ui->inc;
		ui->bl += ui->inc;
	}
}

static void up_pad(struct ui *ui)
{
	if (ui->pad_y > 0) 
	{
		ui->pad_y -= ui->inc;
		ui->tl -= ui->inc;
		ui->bl -= ui->inc;
	}
}

static void minus_len(int len[PLINE], int y)
{
	if (len[y] > 0)
		--len[y];	
}

static void add_len(int len[PLINE], int y, int c)
{
	if ((len[y] + c) < NCOL)
		len[y] += c;
}

static void diary_write(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac)
{
	int my = 0;
	int y = 0;
	
	char buf[PLINE][PCOL] = {0};
	int len[PLINE] = {0};
	bool wf = FALSE;
	int ts = 4;
	
	wmove(ui->pad, y, len[y]);
	prefresh(ui->pad, ui->pad_y, ui->pad_x, ui->scr_y, ui->scr_x, ui->scr_y + ui->scr_h, ui->scr_x + ui->scr_w);
	curs_set(1);
	int ch;
	bool done = FALSE;
	while (!done && (ch = wgetch(ui->pad)) != ERR)
	{
		if (y >= ui->bl)
			down_pad(ui);
		else if (y <= ui->tl)
			up_pad(ui);
	
		wf = FALSE;
		if (ch > 31 && ch < 127)
		{
			buf[y][len[y]] = ch;
			add_len(len, y, 1);
			if (ch > 31)
				wf = TRUE;
		}
		if (y > my)
			my = y;
		switch(ch)
		{
			case CTRLE:
				down_pad(ui);
				break;
			case CTRLY:
				up_pad(ui);
				break;
			case KEY_DOWN:
				down_line(&y);
				break;
			case KEY_UP:
				up_line(&y);
				break;
			case '\n':
				down_line(&y);
				break;
			case '\t':
				if (len[y]+ts < NCOL)
				{
					for (int i = 0; i < ts; ++i)
						buf[y][len[y]+i] = ' ';
				}
				add_len(len, y, ts);
				break;
			case 127: case 8: case KEY_BACKSPACE:
				if (y == 0 && len[y] == 0)
					break;
			
				for (int j = len[y]; j < OCOL; j++)
				{
					if (buf[y][j] == '\0')
						break;
					buf[y][j] = buf[y][j+1];
				}
				if (len[y]-- <= 0)
				{
					len[y] = 0;
					up_line(&y);
					minus_len(len, y);
					break;
				}
				break;
			case 27:
				done = TRUE;
				break;
		}
		if (len[y] == CCOL && wf == TRUE)
		{
			int i, c = 0;
			for (i = len[y]; !isblank((unsigned char)buf[y][i-1]); i--, c++)
			{
				if (c == CCOL)
					break;
			}
			if (c < CCOL-2)
			{
				if (len[y+1] > 0 && my+1 < OLINE)
				{
					my++;
					for (int j = my; j > y; j--)
					{
						memcpy(buf[j+1], buf[j], PCOL);
						len[j+1] = len[j];
					}
					memset(buf[y+1], 0, PCOL);
					len[y+1] = 0;
				}
				
				c = 0;
				for (;i < CCOL; i++, c++)
				{
					if (buf[y+1][c] == '\0')
						buf[y+1][c] = ' ';
					buf[y+1][c] = buf[y][i];
					buf[y][i] = 0;
					minus_len(len, y);
				}
	
				end_line(buf, len, y);
				down_line(&y);
				add_len(len, y, c);
			}
		}
		if (len[y] == CCOL)
		{
			end_line(buf, len, y);
			down_line(&y);
		}
	
		end_line(buf, len, y);
			
		werase(ui->pad);
		for (int ty = 0; ty <= my; ty++)
			mvwprintw(ui->pad, ty, 0, "%s", buf[ty]);
	
		wmove(ui->pad, y, len[y]);
		prefresh(ui->pad, ui->pad_y, ui->pad_x, ui->scr_y, ui->scr_x, ui->scr_y + ui->scr_h, ui->scr_x + ui->scr_w);
	}
	curs_set(0);
	
	for(int j = 0; j <= my; ++j)
	{
		if (buf[j][0] == '\n' && buf[j+1][0] < 31)
			buf[j][0] = '\0';
	}
	
	char buf1[PMAX] = {0};
	int n = 0;
	for (int ty = 0; ty <= my; ++ty)
	{
		for(int z = 0; n < (PCOL + (PCOL * y)); ++n, ++z)
		{
			if (buf[ty][z] == '\0')
				break;
			buf1[n] = buf[ty][z];
		}
	}
	buf1[n] = '\0';
	
	save_diary(cdata, ui, buf1, n);
	
	ui_resize(cdata, pxx, ui, planet, zodiac, FALSE);
	delwin(ui->pad);
}

static void print_menu(struct cdata *cdata, char *path)
{
	char *diary_path = ecalloc(1, MAXPATH);
	size_t size = file_count(path, 1) + 1;
	
	ITEM **item = ecalloc(size, sizeof(ITEM *));
	char **name = ecalloc(size, sizeof(char *));
	char **desc = ecalloc(size, sizeof(char *));
	
	size_t icount = name_to_item(path, item, name, desc);
	
	int header = 4;
	int width = 25;
	int height = (int)icount + header;
	if (icount > 16)
		height = 16 + header;
	
	int starty = (LINES - height) / 2;
	int startx = (COLS - width) / 2;
	
	WINDOW *win = newwin(height, width, starty, startx);
	WINDOW *subwin = derwin(win, height - header, width - 2, 3, 1);
	MENU *menu = new_menu(item);
	FILE *fp;
	
	wbkgdset(win, COLOR_PAIR(M_COLOR));
	keypad(win, TRUE);
	box(win, 0, 0);
	
	menu_opts_off(menu, O_NONCYCLIC);
	menu_opts_off(menu, O_SHOWDESC);
	set_menu_back(menu, COLOR_PAIR(M_COLOR));
	set_menu_fore(menu, COLOR_PAIR(M_COLOR) | A_REVERSE);
	set_menu_win(menu, win);
	set_menu_sub(menu, subwin);
	
	mvwprintw(win, 1, 1, " /diary");
	mvwprintw(win, 1, width - 4, "{l}");
	mvwhline(win, 2, 1, ACS_HLINE, width - 2);
	
	post_menu(menu);
	wrefresh(win);
	
	struct stat st;
	const char *selected = NULL;
	ITEM *cur = NULL;
	
	char buffer[MAXBUF] = {0};
	char field[FMAX][MAXBUF] = {0};
	char newpath[MAXPATH] = {0};
	char cur_dir[MAXBUF] = {0};
	snprintf(cur_dir, MAXBUF, " /diary");
	
	char *endptr = NULL;
	long lret;
	double dret;
	errno = 0;
	
	int ch = 0, count = 0, menu_done = 0;
	while (!menu_done && (ch = wgetch(win)))
	{
		cur = current_item(menu);
		selected = item_description(cur);
		
		switch(ch)
		{
			case 'j': case KEY_DOWN:
				menu_driver(menu, REQ_DOWN_ITEM);
				break;
			case 'k': case KEY_UP:
				menu_driver(menu, REQ_UP_ITEM);
				break;
			case 'l': case KEY_RIGHT: case '\n':
				if ((int)icount == 0)
					break;
				
				int n = snprintf(newpath, MAXPATH, "%s/%s", path, selected);
				if (n >= MAXPATH)
					ERR_EXIT("diary newpath too long");
		
				if (stat(newpath, &st) == 0 && S_ISDIR(st.st_mode))
				{
					memcpy(path, newpath, strlen(newpath) + 1);
					snprintf(cur_dir, MAXBUF, " /%s", selected);
					break;
				}
				
				snprintf(cdata->chart_name, MAXBUF, "%s", selected);
				
				fp = fopen(newpath, "r");
				if (fp == NULL)
					ERR_EXIT("diary newpath fopen");
				
				if (fgets(buffer, MAXBUF, fp) != NULL)
				{
					char *it = strtok(buffer, ",");
				
					while(it)
					{
						snprintf(field[count++], MAXBUF, "%s", it);
						it = strtok(NULL, ",");
					}
				}
				
				memcpy(cdata->city, field[FCITY], strlen(field[FCITY]) + 1);
				memcpy(cdata->state, field[FSTATE], strlen(field[FSTATE]) + 1);
				memcpy(cdata->country, field[FCOUNTRY], strlen(field[FCOUNTRY]) + 1);
					
				lret = strtol(field[FYEAR], &endptr, 10);
				if (errno != ERANGE)
					cdata->year = (int)lret;
				else
					cdata->year = 1970;
				errno = 0;
							
				lret = strtol(field[FMONTH], &endptr, 10);
				if (errno != ERANGE && lret != -1)
					cdata->mon = (int)lret;
				else
					cdata->mon = 1;
				errno = 0;
							
				lret = strtol(field[FDAY], &endptr, 10);
				if (errno != ERANGE && lret != -1)
					cdata->mday = (int)lret;
				else
					cdata->mday = 1;
				errno = 0;
							
				lret = strtol(field[FHOUR], &endptr, 10);
				if (errno != ERANGE && lret != -1) 
					cdata->hour = (int)lret;
				else
					cdata->hour = 1;
				errno = 0;
								
				lret = strtol(field[FMIN], &endptr, 10);
				if (errno != ERANGE && lret != -1)
					cdata->min = (int)lret;
				else
					cdata->min = 1;
				errno = 0;
					
				lret = strtol(field[FSEC], &endptr, 10);
				if (errno != ERANGE && lret != -1)
					cdata->sec = (int)lret;
				else
					cdata->sec = 0;
				errno = 0;
							
				if (setenv("TZ", field[FTZ], 1) != 0)
					ERR_EXIT("ERR: dairy print_menu TZ setenv");
				tzset();
				memcpy(cdata->timezone, field[FTZ], strlen(field[FTZ]) + 1);
						
				dret = strtod(field[FLAT], &endptr);
				if (errno != ERANGE)
					cdata->dlat = dret;
				else
					cdata->dlat = 0.0;
				errno = 0;
						
				dret = strtod(field[FLON], &endptr);
				if (errno != ERANGE)
					cdata->dlon = dret;
				else
					cdata->dlon = 0.0;
				errno = 0;
				
				lret = strtol(field[FDST], &endptr, 10);
				if (lret > 0)
					cdata->isdst = YDST;
				else if (lret == 0)
					cdata->isdst = NDST;
				errno = 0;
					
				fclose(fp);
				
				menu_done = 1;
				break;
			case 'h': case KEY_LEFT:
				if (strcmp(path, diary_path) == 0)
					break;
				back_dir(path, cur_dir);
				break;
			case 'q': case 27:
				menu_done = 1;
				break;
		}
		if (ch == 'h' || ch == 'l' || ch == '\n' || ch == KEY_LEFT || ch == KEY_RIGHT || ch == 'm')
		{
			unpost_menu(menu);
			set_menu_items(menu, NULL);
			for (size_t i = 0; i < icount; ++i)
			{
				free_item(item[i]);
				free(name[i]);
				free(desc[i]);
			}
			
			werase(win);
			wnoutrefresh(win);
			mvwprintw(win, 1, 1, "%s", cur_dir);
			mvwprintw(win, 1, width - 4, "{l}");
			mvwhline(win, 2, 1, ACS_HLINE, width - 2);
			
			icount = name_to_item(path, item, name, desc);
			set_menu_items(menu, item);
			
			width = 25;
			height = (int)icount + header;
			if (icount > 16)
				height = 16 + header;
			
			starty = (LINES - height) / 2;
			startx = (COLS - width) / 2;
			
			wresize(win, height, width);
			wresize(subwin, height - header, width -2);
			mvwin(win, starty, startx);
			mvwin(subwin, starty, startx);
		}
		box(win, 0, 0);
		post_menu(menu);
		doupdate();
	}
	memcpy(path, newpath, strlen(newpath)+1);
	
	unpost_menu(menu);
	free_menu(menu);
	for (size_t j = 0; j < icount; ++j)
	{
		free_item(item[j]);
		free(name[j]);
		free(desc[j]);
	}
	free(name);
	free(desc);
	free(item);
	
	werase(win);
	wrefresh(win);
	delwin(subwin);
	delwin(win);
}

static void diary_read(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac)
{
	FILE *fp;
	size_t bm = PLINE;
	char *buf = NULL;
	int y = 0;
	int ly = 1;
	
	char path[MAXPATH];
	path_check(path, "diary");
	
	print_menu(cdata, path);
	diary_path(path, cdata->year, cdata->mon, cdata->mday);
	
	fp = fopen(path, "r");
	if (!fp)
		ERR_EXIT("read_diary fopen");
	ssize_t n;
	mvwprintw(ui->pad, 0, 0, "%.*s", PCOL, path);
	while ((n = getline(&buf, &bm, fp)) != -1)
	{
		mvwprintw(ui->pad, ly, 0, "%s", buf);
		ly++;
		if (ly == PLINE)
			wresize(ui->pad, bm, PCOL);
	}
	
	new_chart(cdata, pxx, ui, planet, zodiac);
	wrefresh(ui->main_win);
	prefresh(ui->pad, ui->pad_y, ui->pad_x, ui->scr_y, ui->scr_x, ui->scr_y + ui->scr_h, ui->scr_x + ui->scr_w);
	
	curs_set(1);
	int ch;
	bool done = FALSE;
	while (!done && (ch = wgetch(ui->pad)) != ERR)
	{
		if (y >= ui->bl)
			down_pad(ui);
		else if (y <= ui->tl)
			up_pad(ui);
	
		switch(ch)
		{
			case CTRLE:
				down_pad(ui);
				y+= ui->inc;
				break;
			case CTRLY:
				up_pad(ui);
				y-= ui->inc;
				break;
			case KEY_DOWN: case 'j':
				down_line(&y);
				break;
			case KEY_UP: case 'k':
				up_line(&y);
				break;
			case '\n':
				down_line(&y);
				break;
			case 'q': case 27:
				done = TRUE;
				break;
		}
		wmove(ui->pad, y, 0);
		prefresh(ui->pad, ui->pad_y, ui->pad_x, ui->scr_y, ui->scr_x, ui->scr_y + ui->scr_h, ui->scr_x + ui->scr_w);
	}
	curs_set(0);
	free(buf);
}

void diary(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac, int ch)
{
	int year = cdata->year;
	int mon = cdata->mon;
	int day = cdata->mday;
	int hour = cdata->hour;
	int min = cdata->min;
	int sec = cdata->sec;
	char *tz = cdata->timezone;
	double lat = cdata->dlat;
	double lon = cdata->dlon;
	
	ui_resize(cdata, pxx, ui, planet, zodiac, TRUE);
	status_bar(ui, WATER, "diary");
	wrefresh(ui->main_win);
	
	ui->pad = newpad(PLINE, PCOL);
	wbkgdset(ui->pad,COLOR_PAIR(M_COLOR));
	keypad(ui->pad, TRUE);
	
	// upper left corner in pad
	ui->pad_y = 0; 
	ui->pad_x = 0;
	
	// where on screen to start drawing
	ui->scr_y = 2; 
	ui->scr_x = COLS - PCOL;
	
	// height of displayed region
	ui->scr_h = LINES - 4;
	ui->scr_w = PCOL; // width

	ui->inc = 4;
	ui->tl = 0;
	ui->bl = ui->scr_h;
	
	if (ch == 'j')
		diary_write(cdata, pxx, ui, planet, zodiac);
	if (ch == 'J')
		diary_read(cdata, pxx, ui, planet, zodiac);
		
	cdata_clear(cdata);
	cdata->year = year;
	cdata->mon = mon;
	cdata->mday = day;
	cdata->hour = hour;
	cdata->min = min;
	cdata->sec = sec;
	strncpy(cdata->timezone, tz, MAXBUF);
	cdata->dlat = lat;
	cdata->dlon = lon;
	
	ui_resize(cdata, pxx, ui, planet, zodiac, FALSE);
	delwin(ui->pad);
}


