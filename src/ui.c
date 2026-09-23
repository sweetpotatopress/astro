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

#include <ncurses.h>
#include "astro.h"
#include "init.h"

void ui_resize(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac, bool x)
{
	if (!x)
	{
		ui->old_l = ui->left_trig;
		ui->old_r = ui->right_trig;
	
		int toty, totx;
		getmaxyx(ui->main_win, toty, totx);
		
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
	}

	if (x)
	{
		ui->left_trig = ui->old_l;
		ui->right_trig = ui->old_r;
		wheel_init(ui->main_win, ui, 0, 0, 0);
		new_chart(cdata, pxx, ui, planet, zodiac);
	}
}

void table_trigger(struct ui *ui, int ch)
{
	int up = 0;
	if (ch == 'p' && ui->left_trig == 0)
	{
		ui->left_trig = 1;
		show_panel(ui->left_panel);
		up = 1;
	}
	else if (ch == 'p' && ui->left_trig == 1)
	{
		ui->left_trig = 0;
		hide_panel(ui->left_panel);
		up = 1;
	}
	if (ch == 'o' && ui->right_trig == 0)
	{
		ui->right_trig = 1;
		show_panel(ui->right_panel);
		up = 1;
	}
	else if (ch == 'o' && ui->right_trig == 1)
	{
		ui->right_trig = 0;
		hide_panel(ui->right_panel);
		up = 1;
	}
	if (up)
	{
		update_panels();
		doupdate();
	}
}
