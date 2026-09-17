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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include "swephexp.h"
#include "astro.h"
#include "chronos.h"

struct node {
	char *date;
	struct node **children;
	double jd_ut;
	int sign;
	size_t count;
};

struct node *node_create(const char *date, double jd_ut, int sign)
{
	struct node *node = ecalloc(1, sizeof *node);
	node->date = ecalloc(1, strlen(date) + 1);
	memcpy(node->date, date, strlen(date) +1);
	node->jd_ut = jd_ut;
	node->sign = sign;
	
	node->children = NULL;
	node->count = 0;
	return node;
}

int add_child(struct node *parent, struct node *child)
{
	struct node **new_child = erealloc(parent->children, (parent->count + 1) * sizeof *new_child);
	
	parent->children = new_child;
	parent->children[parent->count] = child;
	parent->count++;
	return 1;
}

void node_print(WINDOW *win, const struct node *node, int *sy, int *sx)
{	
	mvwprintw(win,*sy, *sx, "%s", node->date);
	(*sy)++;
	
	for (size_t i = 0; i < node->count; i++)
		node_print(win, node->children[i], sy, sx);
}	

void node_free(struct node *node)
{
	for (size_t i = 0; i < node->count; i++)
		node_free(node->children[i]);
	free(node->children);
	free(node->date);
	free(node);
}

void zodiacal_releasing(struct cdata *cdata, struct pxx *pxx, struct ui *ui)
{
	WINDOW *win = newwin(30, 80, LINES/2, COLS/2);
	double next_jd = 0.0;
	
	double year = 360;
	double month = year / 12;
	double week = month / 12;
	double day = week / 12;

	int pl_period[] = { 0,
	15, 8, 20, 25,
	19, 20, 8, 15,
	12, 27, 30, 12 };
	
	int sign = (int)(pxx->dfor[LONG] / 30)+ 1;
	
	time_t t = 0;
	struct tm temp = {0};
	struct tm *result = NULL;
	struct cdata *tmp = ecalloc(1, sizeof(*cdata));
	memcpy(tmp, cdata, sizeof(*cdata));
	
	char root_date[32];
	snprintf(root_date, 32, "%s: %d.%d.%d", ui->sym.zo_sym[sign], cdata->year, cdata->mon, cdata->mday);
	
	struct node *l1 = node_create(root_date, 0.0, sign);
	l1->jd_ut = swe_julday(cdata->utc_year, cdata->utc_mon, cdata->utc_mday, cdata->utc_hour, SE_GREG_CAL);
	char date[32];

	for (int i = 0; (next_jd - l1->jd_ut) < 120 * year; ++i)
	{
		tmp->mday += (int)(pl_period[sign] * year);
		if (++sign >= 12)
			sign = 1;
		cpt(tmp, &temp, result, &t, 0);
		
		cpt(tmp, &temp, result, &t, 1);
		
		calculate_utc(tmp);
		
		next_jd = swe_julday(tmp->utc_year, tmp->utc_mon, 
		tmp->utc_mday, tmp->utc_hour, SE_GREG_CAL);
		
		snprintf(date, 32, "%s: %d.%d.%d", ui->sym.zo_sym[sign], tmp->year, tmp->mon, tmp->mday);
		struct node *child = node_create(date, next_jd, sign);
		add_child(l1, child);
	}
	int sy, sx;
	struct node *start = l1;
	struct node *end = l1->children[0];
	int item = 0;
	bool done = 0;
	while (!done)
	{
		werase(win);
		sy = 0;
		sx = 3;
		node_print(win, l1, &sy, &sx);
		sign = start->sign;
		struct node *l2 = node_create(start->date, start->jd_ut, sign);
		int zyear, zmon, zday;
		double zhour;
		swe_revjul(start->jd_ut, SE_GREG_CAL, &zyear, &zmon, &zday, &zhour);
		tmp->year = zyear;
		tmp->mon = zmon;
		tmp->mday = zday;
		next_jd = 0.0;
		
		while(end->jd_ut > next_jd)
		{
			tmp->mday += (int)lround(pl_period[sign] * month);
			if (++sign >= 12)
				sign = 1;
			
			cpt(tmp, &temp, result, &t, 0);
			cpt(tmp, &temp, result, &t, 1);
			calculate_utc(tmp);
			next_jd = swe_julday(tmp->utc_year, tmp->utc_mon, tmp->utc_mday, tmp->utc_hour, SE_GREG_CAL);
			
			snprintf(date, 32, "%s: %d.%d.%d", ui->sym.zo_sym[sign], tmp->year, tmp->mon, tmp->mday);
			struct node *child = node_create(date, next_jd, sign);
			add_child(l2, child);
		}
		
		sy = 0;
		sx += 18;
		node_print(win, l2, &sy, &sx);
		const char *select = "->";
		mvwprintw(win, item, 0, "%s", select);
		wrefresh(win);
		
		int ch = wgetch(win);
		switch(ch)
		{
			case 'j':
				mvwprintw(win, item, 0, "  ");
				item++;
				if ((size_t)item >= l1->count)
					item = 0;
				mvwprintw(win, item, 0, "%s", select);
				wrefresh(win);
				start = (item == 0) ? l1 : l1->children[item - 1];
				end = l1->children[item];

				break;
			case 'k':
				mvwprintw(win, item, 0, "  ");
				if (item > 0)
					--item;
				else
					item = (int)l1->count - 1;
				mvwprintw(win, item, 0, "%s", select);
				wrefresh(win);
				start = (item == 0) ? l1 : l1->children[item - 1];
				end = l1->children[item];

				break;
			default:
				done = 1;
				break;
		}
		node_free(l2);
	}
	werase(win);
	wrefresh(win);
	node_free(l1);
	free(tmp);
	delwin(win);
}
