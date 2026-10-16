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

static void save_diary(struct cdata *cdata, struct ui *ui, char *buf)
{
	if (strlen(buf) < 1)
		return;
		
	struct cdata time = {0};
	d_timezone(ui->conf_timezone);
	set_localtime(&time);
	d_timezone(cdata->timezone);
	
	char filename[MAXBUF] = {0};
	snprintf(filename, MAXBUF, "%02d.%02d.md", time.mon, time.mday);
	
	char path[MAXPATH] = {0};
	path_check(path, "diary");
	
	char year_path[MAXPATH] = {0};
	snprintf(year_path, MAXPATH, "%s/%d", path, time.year);
	
	if (access(year_path, R_OK) == -1)
		if (mkdir(year_path, 0755) == -1)
			ERR_EXIT("ERR: diary mkdir");
		
	char full_path[MAXPATH] = {0};
	snprintf(full_path, MAXPATH, "%s/%s", year_path, filename);
	
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
	
	size_t tmpmax = PLINE;
	char *buf = ecalloc(1, tmpmax * sizeof(char *));
	int i = 0;
	
	int y = 1;
	int x[PLINE] = {0};
	
	int pad_y = 0; // upper left corner in pad
	int pad_x = 0;
	
	int scr_y = 2; // where on screen to start drawing
	int scr_x = COLS - PCOL;
	
	int scr_h = LINES - 4; // height of displayed region
	int scr_w = PCOL; // width
	prefresh(pad, pad_y, pad_x, scr_y, scr_x, scr_y + scr_h, scr_x + scr_w);

	curs_set(1);
	int ch;
	bool done = FALSE;
	while (!done && (ch = wgetch(pad)))
	{
		buf[i] = ch;
		switch(ch)
		{
			case 10:
				y++;
				mvwprintw(pad, y, x[y], "%c", buf[i]);
				i++;
				break;
			case 127: case 8: case KEY_BACKSPACE:
				if (y == 0 && x[y] == 0)
					break;
	
				if (x[y]-- <= 0)
				{
					x[y] = 0;
					if (y-- <= 0)
						y = 0;
				}
				buf[i] = ' ';
				mvwprintw(pad, y, x[y], "%c", buf[i]);
				if (i-- <= 0)
					i = 0;
				break;
			case 27:
				done = TRUE;
				break;
			case KEY_DOWN:
				if (pad_y < PLINE - scr_h -1)
					++pad_y;
				break;
			case KEY_UP:
				if (pad_y > 0)
					--pad_y;
				break;
			default:
				mvwprintw(pad, y, x[y], "%c", buf[i]);
				x[y]++;
				i++;
				break;
		}
		if (x[y] >= PCOL-1 && ch != KEY_BACKSPACE)
		{
			y++;
		}
		if (y > PLINE - 2 || strlen(buf) > PMAX - 2)
		{
			break;
		}
		if((size_t)i >= tmpmax)
		{
			tmpmax *= 2;
			char *tmp = erealloc(buf, tmpmax * sizeof(char *));
			buf = tmp;
		}
	
		wmove(pad, y, x[y]);
		prefresh(pad, pad_y, pad_x, scr_y, scr_x, scr_y + scr_h, scr_x + scr_w);
	
	}
	buf[i] = '\0';
	curs_set(0);
	
	save_diary(cdata, ui, buf);
	
	ui_resize(cdata, pxx, ui, planet, zodiac, FALSE);
	delwin(pad);
	free(buf);
}
