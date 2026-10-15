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

#define PROW 10024
#define PCOL 48

void diary(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac)
{
	ui_resize(cdata, pxx, ui, planet, zodiac, TRUE);
	
	WINDOW *pad = newpad(PROW, PCOL);
	
	wbkgdset(pad,COLOR_PAIR(M_COLOR));
	keypad(pad, TRUE);
	
	char buf[MAXBUF] = {0};
	int i = 0;
	
	int pad_y = 0; // upper left corner in pad
	int pad_x = 0;
	
	int scr_y = 1; // where on screen to start drawing
	int scr_x = COLS - PCOL;
	
	int scr_h = LINES - 2; // height of displayed region
	int scr_w = PCOL; // width
	prefresh(pad, pad_y, pad_x, scr_y, scr_x, scr_y + scr_h, scr_x + scr_w);
	
	int y = 1;
	int x[MAXBUF] = {0};
	
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
				if (x[y]-- <= 0)
				{
					x[y] = 0;
					if (y-- <= 0)
						y = 0;
				}
				buf[i] = ' ';
				mvwprintw(pad, y, x[y], "%c", buf[i]);
				i--;
				break;
			case 27:
				done = TRUE;
				break;
			default:
				mvwprintw(pad, y, x[y], "%c", buf[i]);
				x[y]++;
				i++;
				break;
		}
		if (x[y] >= PCOL-1)
		{
			y++;
		}
		wmove(pad, y, x[y]);
		prefresh(pad, pad_y, pad_x, scr_y, scr_x, scr_y + scr_h, scr_x + scr_w);
	}
	buf[i] = '\0';
	curs_set(0);
	

	struct cdata time = {0};
	set_localtime(&time);
	
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
		snprintf(header, MAXBUF, "%s, %f, %f\n---\n%02d:%02d:%02d", cdata->timezone, cdata->dlon, cdata->dlat,
		time.hour, time.min, time.sec);
	else
		snprintf(header, MAXBUF, "%02d:%02d:%02d", time.hour, time.min, time.sec);
	char *footer = "- - - - - - -";
	
	FILE *fp = fopen(full_path, "a");
	if (!fp)
		ERR_EXIT("ERR: diary fopen");

	fprintf(fp, "%s\n%s\n%s\n", header, buf, footer);
		
	fclose(fp);
	ui_resize(cdata, pxx, ui, planet, zodiac, FALSE);
	delwin(pad);
}
