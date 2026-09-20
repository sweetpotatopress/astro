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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ncurses.h>
#include "swephexp.h"
#include "astro.h"
#include "init.h"
#include "draw.h"
#include "chronos.h"

#define FORTUNE 0
#define SPIRIT 1

#define ROOT -1
#define ZYEAR 0
#define ZMONTH 1
#define ZWEEK 2
#define ZDAY 3
#define ZMAX 4

struct node {
	char *date;

	double jd_ut;
	int sign;
	int level;

	int year;
	int mon;
	int mday;
	int hour;
	int min;
	int sec;
	int isdst;

	struct node **children;
	size_t count;
};

static struct node *node_create(const char *date, double jd_ut, int sign, int level,
int year, int mon, int mday, int hour, int min, int sec, int isdst)
{
	struct node *node = ecalloc(1, sizeof(*node));

	node->date = ecalloc(strlen(date) + 1, sizeof(*node->date));
	memcpy(node->date, date, strlen(date) + 1);

	node->jd_ut = jd_ut;
	node->sign = sign;
	node->level = level;

	node->year = year;
	node->mon = mon;
	node->mday = mday;
	node->hour = hour;
	node->min = min;
	node->sec = sec;
	node->isdst = isdst;

	node->children = NULL;
	node->count = 0;

	return node;
}

static void node_add_child(struct node *parent, struct node *child)
{
	struct node **children = erealloc(parent->children, (parent->count + 1) * sizeof(*children));

	parent->children = children;
	parent->children[parent->count] = child;
	parent->count++;
}

static void node_free(struct node *node)
{
	if (node == NULL)
		return;

	for (size_t i = 0; i < node->count; i++)
		node_free(node->children[i]);

	free(node->children);
	free(node->date);
	free(node);
}

static void advance_date(struct cdata *date, double days)
{
	struct tm temp = {0};
	time_t t = 0;
	time_t seconds;

	cpt(date, &temp, &t, 0);
	
	seconds = (time_t)llround(days * 86400.0);
	t += seconds;
	cpt(date, &temp, &t, 1);
}

static void date_string(char *buffer, size_t buffer_size, struct ui *ui, int year, int mon, int mday, int sign, bool b)
{
	if (b == 1)
		snprintf(buffer, buffer_size, "%s:+%d.%s.%02d", ui->sym.zo_sym[sign], year, ui->sym.month[mon], mday);
	else
		snprintf(buffer, buffer_size, "%s: %d.%s.%02d", ui->sym.zo_sym[sign], year, ui->sym.month[mon], mday);
}

static double node_end(const struct node *node, const double *layer_inc, const int *pl_period)
{
	if (node->level == ROOT) 
		return node->jd_ut + 120 * layer_inc[ZYEAR];

	return node->jd_ut + pl_period[node->sign] *layer_inc[node->level];
}

static void node_generate_children(struct node *parent, struct ui *ui, int child_level)
{
	const double layer_inc[ZMAX] = {
		360.0,
		30.0,
		2.5,
		2.5 / 12
	};

	const int pl_period[13] = {
		0,
		15, 8, 20, 25,
		19, 20, 8, 15,
		12, 27, 30, 12
	};
	
	struct cdata parent_date;
	double parent_end;
	double child_jd;
	double child_length;

	int child_sign;
	int bond_sign;
	bool bond_switch = 0;
	bool bond_print = 0;
	char date[MAXBUF];

	if (parent->count != 0)
		return;

	memset(&parent_date, 0, sizeof(parent_date));

	parent_date.year = parent->year;
	parent_date.mon = parent->mon;
	parent_date.mday = parent->mday;
	parent_date.hour = parent->hour;
	parent_date.min = parent->min;
	parent_date.sec = parent->sec;
	parent_date.isdst = parent->isdst;

	parent_end = node_end(parent, layer_inc, pl_period);
	
	bond_sign = parent->sign;
	child_sign = parent->sign;

	for (;;) 
	{
		child_jd = swe_julday(parent_date.year, parent_date.mon, parent_date.mday, parent_date.utc_hour, SE_GREG_CAL);

		if (child_jd >= parent_end)
			break;

		child_length = pl_period[child_sign] * layer_inc[child_level];

		if (child_length <= 0.0)
			break;

		date_string(date, sizeof(date), ui, parent_date.year, parent_date.mon, parent_date.mday, child_sign, bond_print);
		if (bond_print == 1)
			bond_print = 0;

		node_add_child(parent, 
		node_create(date, child_jd, child_sign, child_level, parent_date.year,
		parent_date.mon, parent_date.mday, parent_date.hour, parent_date.min, parent_date.sec, parent_date.isdst));

		advance_date(&parent_date, child_length);

		++child_sign;
		if (child_sign > 12)
			child_sign = 1;

		if (child_sign == bond_sign && !bond_switch)
		{
			bond_switch = 1;
			bond_print = 1;
			child_sign += 6;
			if (child_sign > 12)
				child_sign -= 12;
		}
	}
}

static struct node *create_root(struct cdata *cdata, struct pxx *pxx, struct ui *ui, int sign_switch)
{
	char date[MAXBUF];
	int sign;

	if (sign_switch == SPIRIT) 
	{
		sign = (int)(pxx->dspir[LONG] / 30.0) + 1;

		if ((int)(pxx->dspir[LONG] / 30.0) == (int)(pxx->dfor[LONG] / 30.0))
			sign++;
	} 
	else 
		sign = (int)(pxx->dfor[LONG] / 30.0) + 1;

	if (sign > 12)
		sign -= 12;
	if (sign < 1)
		sign += 12;

	date_string(date, sizeof(date), ui, cdata->year, cdata->mon, cdata->mday, sign, 0);

	struct node *root = node_create(date, cdata->jd_ut, sign, ROOT,
	cdata->year, cdata->mon, cdata->mday, cdata->hour, cdata->min, cdata->sec, cdata->isdst);

	return root;
}

static void print_column(WINDOW *win, struct ui *ui, const struct node *parent, int selected, int layer, int **zodiac)
{
	const int width = 20;
	const int height = 22;
	
	int x = (layer % 2) * width + 1;
	int ys = (layer / 2) * height;
	int max_y = getmaxy(win);

	for (size_t i = 0; i < parent->count; i++)
	{
		int y = ys + (int)i;
		if (y >= max_y)
			break;
			
		const struct node *child = parent->children[i];
		const char *part = strchr(child->date, ':');
		
		if ((int)i == selected && parent->level < ZWEEK)
			mvwprintw(win, y, x, "-->");
		else
			mvwprintw(win, y, x, "   ");
			
		zo_color(win, ui, y, x+3, child->sign, zodiac);
		mvwprintw(win, y, x+6, "%s", part);
	}
}

static void rebuild_path(struct node **parents, int *selected, struct ui *ui)
{
	int level;

	for (level = 0; level < ZMAX; level++)
	{
		struct node *parent = parents[level];

		node_generate_children(parent, ui, level);

		if (parent->count == 0)
			break;

		if (selected[level] < 0)
			selected[level] = 0;

		if ((size_t)selected[level] >= parent->count)
			selected[level] = (int)parent->count - 1;

		if (level + 1 < ZMAX)
			parents[level + 1] = parent->children[selected[level]];
	}

	for (int i = level + 1; i <= ZMAX; i++)
		parents[i] = NULL;
}

static void move_selection(struct node **parents, int *selected, int layer, int direction)
{
	struct node *parent = parents[layer];

	if (parent == NULL || parent->count == 0)
		return;

	int count = (int)parent->count;

	selected[layer] += direction;

	if (selected[layer] < 0)
		selected[layer] = count - 1;
	else if (selected[layer] >= count)
		selected[layer] = 0;

	for (int i = layer + 1; i < ZMAX; i++)
		selected[i] = 0;
}

void zodiacal_releasing(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac)
{
	int height = 47;
	int width = 43;
	int sy = 0;
	int sx = (COLS - width);
	
	int old_l = ui->left_trig;
	int old_r = ui->right_trig;
	
	int toty, totx;
	getmaxyx(ui->main_win, toty, totx);
	
	if (toty < 47)
	{
		printw("window too small for zr");
		getch();
		return;
	}
	if (totx > 163)
	{
		ui->cx = (ui->radius + 35);
		if (toty > 50)
		{
			wheel_init(ui->main_win, ui, 7, 0, 0);
			ui->cx = (ui->radius + 40);
		}
	}
	else
	{
		ui->cx = (ui->radius + 5);
		ui->left_trig = 0;
		ui->right_trig = 0;
	
		hide_panel(ui->left_panel);
		hide_panel(ui->right_panel);
	}
	new_chart(cdata, pxx, ui, planet, zodiac);

	int sign_switch = FORTUNE;
	int current_layer = ZYEAR;
	
	WINDOW *win = newwin(height, width, sy, sx);
	WINDOW *subwin = derwin(win, height - 3, width - 2, 3, 1);
	
	wbkgdset(win,COLOR_PAIR(M_COLOR));
	keypad(win, TRUE);

	struct node *root = create_root(cdata, pxx, ui, sign_switch);
	if (!root)
		ERR_EXIT("zr: failed to create root");
		
	struct node *parents[ZMAX] = {0};
	int selected[ZMAX] = {0};

	parents[0] = root;

	bool done = 0;
	while (!done) 
	{
		rebuild_path(parents, selected, ui);

		werase(win);
		werase(subwin);

		for (int layer = ZYEAR; layer <= ZDAY; layer++)
			print_column(subwin, ui, parents[layer], selected[layer], layer, zodiac);

		mvwprintw(win, 1, 5, "layer: { ");
		wattron(win, COLOR_PAIR(current_layer+3));
		mvwprintw(win, 1, 5+9, "%d", current_layer+1);
		wattroff(win, COLOR_PAIR(current_layer+3));
		mvwprintw(win, 1, 5+11, "}");
		
		const char *lot = sign_switch == SPIRIT ? "spirit" : "fortune";
		mvwprintw(win, 1, width - 17, "[tab] %s", lot);
		
		// highlight current layer
		mvwhline(win, 2, 1, ACS_HLINE, width - 2);
		mvwhline(win, 24, 1, ACS_HLINE, width - 2);
		
		wattron(win, COLOR_PAIR(current_layer+3));
		
		mvwhline(win, 2, 1, ACS_HLINE, (current_layer +1) * 21);
		if (current_layer > ZMONTH)
			mvwhline(win, 24, 1, ACS_HLINE, width - 20);
		wattroff(win, COLOR_PAIR(current_layer+3));
		
		mvwhline(win, 2, 1, ACS_HLINE, (current_layer * 20));

		box(win, 0, 0);
		wnoutrefresh(win);
		update_panels();
		doupdate();
	
		int ch = wgetch(win);
		if (isdigit(ch) && (ch - '0') < ZMAX)
		{
			current_layer = (ch - '0') - 1;
			move_selection(parents, selected, current_layer, 0);
		}
		
		if (ch == 'p' && ui->left_trig == 0)
		{
			ui->left_trig = 1;
			show_panel(ui->left_panel);
			update_panels();
			doupdate();
		}
		else if (ch == 'p' && ui->left_trig == 1)
		{
			ui->left_trig = 0;
			hide_panel(ui->left_panel);
			update_panels();
			doupdate();
		}
		if (ch == 'o' && ui->right_trig == 0)
		{
			ui->right_trig = 1;
			show_panel(ui->right_panel);
			update_panels();
			doupdate();
		}
		else if (ch == 'o' && ui->right_trig == 1)
		{
			ui->right_trig = 0;
			hide_panel(ui->right_panel);
			update_panels();
			doupdate();
		}

		switch (ch)
		{
			case 'j': case KEY_DOWN:
				move_selection(parents, selected, current_layer, +1);
				break;

			case 'k': case KEY_UP:
				move_selection(parents, selected, current_layer, -1);
				break;

			case 'l': case KEY_RIGHT:
				if (current_layer < ZWEEK)
					current_layer++;
				break;

			case 'h': case KEY_LEFT:
				if (current_layer > ZYEAR)
					current_layer--;
				break;

			case '\t':
				sign_switch = sign_switch == FORTUNE ? SPIRIT : FORTUNE;

				node_free(root);

				memset(parents,0,sizeof(parents));
				memset(selected, 0, sizeof(selected));

				root = create_root(cdata, pxx, ui, sign_switch);
				if (!root)
					ERR_EXIT("zr: failed to create root");

				parents[0] = root;
				current_layer = ZYEAR;
				break;

			case 'z': case 'q': case 27:
				done = 1;
				break;
		}
	}
	node_free(root);
	delwin(subwin);
	delwin(win);
	ui->left_trig = old_l;
	ui->right_trig = old_r;
	wheel_init(ui->main_win, ui, 0, 0, 0);
}
