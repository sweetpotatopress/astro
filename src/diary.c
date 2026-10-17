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
#define PCOL 48

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
	for(; buf[i] != '\0' && isspace((unsigned char)buf[i]); ++i)
		;
	if (i == size || buf[i] == '\0')
		return;
		
	struct cdata time = {0};
	d_timezone(ui->conf_timezone);
	set_localtime(&time);
	d_timezone(cdata->timezone);
	
	char filename[12] = {0};
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

	fprintf(fp, "%s\n%s\n%s\n", header, buf, footer);
		
	fclose(fp);
}

int count_elements(int y, int *x)
{
	int count = 0;
	for (int j = 0; j <= y; ++j)
		count += x[j];
	return count;
}

struct bx {
	char c;
	int y;
	int x;
};

void diary(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac)
{
	ui_resize(cdata, pxx, ui, planet, zodiac, TRUE);
	
	WINDOW *pad = newpad(PLINE, PCOL);
	
	wbkgdset(pad,COLOR_PAIR(M_COLOR));
	keypad(pad, TRUE);
	
	wattron(ui->main_win, COLOR_PAIR(WATER));
	mvwprintw(ui->main_win, 0, COLS - 14, "diary");
	wattroff(ui->main_win, COLOR_PAIR(WATER));
	wrefresh(ui->main_win);
	
	const int col_max = PCOL-1;
	size_t tmpmax = PLINE;
	
	char *buf = ecalloc(tmpmax, sizeof*buf);
	int i = 0;
	int n = 0;
	
	struct bx **a = ecalloc(tmpmax, sizeof(*a));
	for (size_t j = 0; j < tmpmax; j++)
		a[j] = ecalloc(1, sizeof(*a[j]));
	
	int my = 1;
	int y = 1;
	int x[PLINE] = {0};
	
	int pad_y = 0; // upper left corner in pad
	int pad_x = 0;
	
	int scr_y = 2; // where on screen to start drawing
	int scr_x = COLS - PCOL;
	
	int scr_h = LINES - 4; // height of displayed region
	int scr_w = PCOL; // width
	wmove(pad, y, x[y]);
	prefresh(pad, pad_y, pad_x, scr_y, scr_x, scr_y + scr_h, scr_x + scr_w);

	curs_set(1);
	int ch;
	bool done = FALSE;
	while (!done && (ch = wgetch(pad)) != ERR)
	{
		if (isascii(ch))
		{
			buf[n] = ch;
			a[n]->c = ch;
			a[n]->y = y;
			a[n]->x = x[y];
			if (y > my)
				my = y;
			++n;
		}
		switch(ch)
		{
			case KEY_DOWN:
				if (y < PLINE)
					++y;
				break;
			case KEY_UP:
				if (y > 1)
					--y;
				break;
			case KEY_LEFT:
				if (x[y] > 0)
					--x[y];
				else if (x[y] == 0 && y > 1)
					--y;
				break;
			case KEY_RIGHT:
				if (x[y] < col_max)
					++x[y];
				else if (x[y] >= col_max)
				{
					x[y] = col_max;
					++y;
				}
				break;
			case 10:
				y++;
				x[y] = 0;
				i++;
				break;
			case 127: case 8: case KEY_BACKSPACE:
				if (y == 1 && x[y] == 0)
					break;
				if (i == 0)
					break;
	
				if (x[y]-- <= 0)
				{
					x[y] = 0;
					if (y-- <= 1)
						y = 1;
					break;
				}
				
				// calculate cursor buf position
				int cy = (col_max * y);
				int cx = (col_max - x[y]);
				int re = (cy - cx) + 1;
				for (int j = re; j < cy; ++j)
				{
					buf[j-1] = buf[j];
				}
					
				for (int j = re; j < cy; j++)
				{
					a[j-1]->c = a[j]->c;
					a[j-1]->x = a[j]->x;
					a[j-1]->y = a[j]->y;
				}
				--n;
				break;
			default:
				i++;
				x[y]++;
			
				break;
			case 27:
				done = TRUE;
				break;
	
		}
		werase(pad);
		int z = count_elements(y, x);
		mvwprintw(pad, 0, 0, "                                       ");
		mvwprintw(pad, 0, 0, "n: %d, z: %d, x[y]: %d, y: %d", n, z, x[y], y);
		if (x[y] > col_max && ch != KEY_BACKSPACE && ch != KEY_LEFT)
		{
			y++;
		}
		if (y > PLINE - 2)
		{
			break;
		}
		if((size_t)i >= tmpmax)
		{
			tmpmax *= 2;
			char *tmp0 = erealloc(buf, tmpmax * sizeof*buf);
			buf = tmp0;
			struct bx **tmp1 = erealloc(a, tmpmax * sizeof*a);
			a = tmp1;
			for (size_t j = 0; j < tmpmax; j++)
			{
				tmp1[j] = erealloc(a[j], tmpmax * sizeof(*a[j]));
				a[j] = tmp1[j];
			}
		}
		
		mvwprintw(pad, 1, 0, "%s", buf);
		wmove(pad, y, x[y]);
		prefresh(pad, pad_y, pad_x, scr_y, scr_x, scr_y + scr_h, scr_x + scr_w);
	
	}
	curs_set(0);
	
	save_diary(cdata, ui, buf, i);
	
	ui_resize(cdata, pxx, ui, planet, zodiac, FALSE);
	delwin(pad);
	free(buf);
}
