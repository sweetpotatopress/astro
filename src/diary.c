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
	int n = snprintf(filename, 12, "%02d.md", mday);
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
			ERR_EXIT("ERR: year_path mkdir");
			
	char mon_path[MAXPATH] = {0};
	n = snprintf(mon_path, MAXPATH, "%s/%d", year_path, mon);
	if (n > MAXPATH)
		ERR_EXIT("ERR: mon_path too long");
		
	if (access(mon_path, R_OK) == -1)
		if (mkdir(mon_path, 0755) == -1)
			ERR_EXIT("ERR: mon path mkdir");
		
	n = snprintf(full_path, MAXPATH, "%s/%s", mon_path, filename);
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
		snprintf(header, MAXBUF, "%s,%s,%s,%d,%d,%d,%d,%d,%d,%s,%f,%f\n- - - - - - -\n%02d:%02d:%02d%s",
		cdata->city, cdata->state, cdata->country,
		time.year, time.mon, time.mday,
		time.hour, time.min, time.sec, 
		ui->conf_timezone, ui->conf_dlat, ui->conf_dlon,
		th, time.min, time.sec, tod);
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

struct ct {
	int hour;
	int min;
	int sec;
	char y;
};

static void next_entry(struct cdata *cdata, struct ct **ct, int *c, int cm, int sign)
{
	*c += sign;
	if (*c >= cm)
		*c = cm;
	else if (*c < 0)
		*c = 0;
		
	cdata->hour = ct[*c]->hour;
	cdata->min = ct[*c]->min;
	cdata->sec = ct[*c]->sec;
}

static void diary_read(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac)
{
	FILE *fp;
	size_t bm = PLINE;
	char *buf = NULL;
	int y = 0;
	int ly = 1;
	int ent_max = 1024;	
	
	struct ct **ct = ecalloc(ent_max, sizeof(*ct));
	for (int i = 0; i < ent_max; ++i)
		ct[i] = ecalloc(1, sizeof(*ct[i]));
	
	char path[MAXPATH];
	path_check(path, "diary");
	
	print_menu(cdata, ui, path, "/diary", 'l', LOAD);
	if (ui->fb_cancel == TRUE)
	{
		ui->fb_cancel = FALSE;
		return;
	}
	
	fp = fopen(path, "r");
	if (!fp)
		ERR_EXIT("read_diary fopen");
	ssize_t n;
	mvwprintw(ui->pad, 0, 0, "%.*s", PCOL, path);

	bool s = FALSE;
	int c = 0;
	while ((n = getline(&buf, &bm, fp)) != -1)
	{
		mvwprintw(ui->pad, ly, 0, "%s", buf);
		
		if (s == TRUE)
		{
			char hour[3];
			char min[3];
			char sec[3];
			hour[0] = buf[0];
			hour[1] = buf[1];
			hour[2] = '\0';
			min[0] = buf[3];
			min[1] = buf[4];
			min[2] = '\0';
			sec[0] = buf[6];
			sec[1] = buf[7];
			sec[2] = '\0';
			
			ct[c]->hour = atoi(hour);
			if (buf[8] == 'P')
			{
				ct[c]->hour += 12;
				if (ct[c]->hour == 24)
					ct[c]->hour = 0;
			}
			ct[c]->min = atoi(min);
			ct[c]->sec = atoi(sec);
			ct[c]->y = ly;
			s = FALSE;
			c++;
		}
			
		if (buf[0] == '-' && buf[2] == '-' && buf[4] == '-')
		{
			s = TRUE;
		}
			
		ly++;
		if (ly == PLINE)
			wresize(ui->pad, bm, PCOL);
	}
	
	int cm = c-1;
	int dif = LINES / 2;
	
	c = 0;
	y = ct[0]->y;
	
	wmove(ui->pad, y, 0);
	
	new_chart(cdata, pxx, ui, planet, zodiac);
	status_bar(ui, WATER, "diary");
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
			case 'l':
				next_entry(cdata, ct, &c, cm, 1);
				
				y = ct[c]->y;
				while(y+dif > ui->bl)
					down_pad(ui);
				new_chart(cdata, pxx, ui, planet, zodiac);
				break;
			case 'h':
				next_entry(cdata, ct, &c, cm, -1);
				
				y = ct[c]->y;	
				int tmp = y;
				tmp -= dif;
				if (tmp < 0)
					tmp = 0;
				while(tmp < ui->tl)
					up_pad(ui);
				new_chart(cdata, pxx, ui, planet, zodiac);
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
		curs_set(1);
		status_bar(ui, WATER, "diary");
	
		wmove(ui->pad, y, 0);
		wrefresh(ui->main_win);
		prefresh(ui->pad, ui->pad_y, ui->pad_x, ui->scr_y, ui->scr_x, ui->scr_y + ui->scr_h, ui->scr_x + ui->scr_w);
	}
	curs_set(0);
	
	for (int i = 0; i < ent_max; ++i)
		free(ct[i]);
	free(ct);
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
	char tz[MAXBUF];
	memcpy(tz, cdata->timezone, MAXBUF);
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


