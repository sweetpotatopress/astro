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
#include <ui.h>
#include <swephexp.h>
#include <chronos.h>

#define PLINE 2560
#define OLINE (PLINE-1)
#define PCOL 48
#define NCOL (PCOL-2)
#define OCOL (PCOL-1)

#define PMAX (PLINE * PCOL)

static void d_timezone(char *timezone)
{
	if (setenv("TZ", timezone, 1) != 0)
		ERR_EXIT("ERR: setenv diary.c");
	tzset();
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
	
	char filename[12] = {0};
	//int n = snprintf(filename, 12, "test.md");
	int n = snprintf(filename, 12, "%02d.%02d.md", time.mon, time.mday);
	if (n > 12)
		ERR_EXIT("ERR: diary filename too long");
	
	char path[MAXPATH] = {0};
	path_check(path, "diary");
	
	char year_path[MAXPATH] = {0};
	n = snprintf(year_path, MAXPATH, "%s/%d", path, time.year);
	if (n > MAXPATH)
		ERR_EXIT("ERR: diary.c year_path too long");
	
	if (access(year_path, R_OK) == -1)
		if (mkdir(year_path, 0755) == -1)
			ERR_EXIT("ERR: diary mkdir");
		
	char full_path[MAXPATH] = {0};
	n = snprintf(full_path, MAXPATH, "%s/%s", year_path, filename);
	if (n > MAXPATH)
		ERR_EXIT("ERR: diary.c full_path too long");
	
	char header[MAXBUF] = {0};
	if (access(full_path, R_OK) == -1)
		snprintf(header, MAXBUF, "%s, %f, %f\n%d.%d.%d\n%02d:%02d:%02d", 
		cdata->timezone, cdata->dlon, cdata->dlat,
		time.year, time.mon, time.mday,
		time.hour, time.min, time.sec);
	else
		snprintf(header, MAXBUF, "%02d:%02d:%02d", time.hour, time.min, time.sec);
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
		
	buf[y][NCOL] = '\n';
	buf[y][OCOL] = '\0';
}

void diary(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac)
{
	ui_resize(cdata, pxx, ui, planet, zodiac, TRUE);
	
	WINDOW *pad = newpad(PLINE, PCOL);
	wbkgdset(pad,COLOR_PAIR(M_COLOR));
	keypad(pad, TRUE);
	
	// upper left corner in pad
	int pad_y = 0; 
	int pad_x = 0;
	
	// where on screen to start drawing
	int scr_y = 2; 
	int scr_x = COLS - PCOL;
	
	// height of displayed region
	int scr_h = LINES - 4;
	int scr_w = PCOL; // width
	
	status_bar(ui, WATER, "diary");
	wrefresh(ui->main_win);
	
	char buf[PLINE][PCOL] = {0};
	int len[PLINE] = {0};
	bool wf = FALSE;
	
	// cursor
	int my = 0;
	int y = 0;
	int x[PLINE] = {0};
	
	wmove(pad, y, x[y]);
	prefresh(pad, pad_y, pad_x, scr_y, scr_x, scr_y + scr_h, scr_x + scr_w);

	curs_set(1);
	int ch;
	bool done = FALSE;
	while (!done && (ch = wgetch(pad)) != ERR)
	{
		if (isalnum(ch) || isspace(ch) || ispunct(ch) || isblank(ch))
		{
			if (len[y] >= NCOL)
				y++;
			if (y > OLINE)
				y = OLINE;
			buf[y][len[y]] = ch;
			wf = FALSE;
		}
		if (isalnum(ch) || ispunct(ch) || isblank(ch))
		{
			len[y]++;
			x[y]++;	
		}
		if (isalnum(ch) || ispunct(ch))
			wf = TRUE;
			
		if (y > my)
			my = y;
		switch(ch)
		{
			case KEY_DOWN:
				if (y < PLINE)
					y++;
				break;
			case KEY_UP:
				if (y > 0)
					y--;
				break;
			case 10:
				y++;
				x[y] = len[y];
				break;
			case 127: case 8: case KEY_BACKSPACE:
				if (y == 0 && x[y] == 0)
					break;
	
				if (x[y]-- <= 0)
				{
					x[y] = 0;
					if (y-- <= 0)
						y = 0;
					break;
				}
				
				int j;
				for (j = x[y]; j < OCOL; j++)
				{
					if (buf[y][j] == '\0')
						break;
					buf[y][j] = buf[y][j+1];
				}
				
				if (--len[y] <= 0)
					len[y] = 0;
				break;
			case 27:
				done = TRUE;
				break;
	
		}
		if (len[y] == NCOL && wf == TRUE)
		{
			int i, c = 0;
			for (i = len[y]; !isblank((unsigned char)buf[y][i]); i--)
				;
			++i;
			for (;i < NCOL; i++, c++)
			{
				buf[y+1][c] = buf[y][i];
				buf[y][i] = 0;
				len[y]--;
			}
				
			end_line(buf, len, y);
			y++;
			len[y] += c;
			x[y] += c;
		}
				
		end_line(buf, len, y);
		
		if (y >= OLINE)
			break;
			
		werase(pad);
		
		//mvwprintw(pad, 10, 0, "len[y]: %d, len[y+1]: %d", len[y], len[y+1]);
		for (int ty = 0; ty <= my; ty++)
			mvwprintw(pad, ty, 0, "%s", buf[ty]);
		
		wmove(pad, y, x[y]);
		prefresh(pad, pad_y, pad_x, scr_y, scr_x, scr_y + scr_h, scr_x + scr_w);
	}
	curs_set(0);
	
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
	delwin(pad);
}
