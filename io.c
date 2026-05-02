/* Copyright (C) 2026 yam lynn
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by the 
any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTIBILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. if not, see <https://www.gnu.org/licenses/> */

#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <ncurses.h>
#include <form.h>
#include <menu.h>
#include "astro.h"


void save_chart(struct tm *cdata, Location *loc, Io *io)
{
	WINDOW *data_dir_win;
	WINDOW *data_dir_subwin;
	FORM *data_dir_form;
	FIELD *data_dir_field[2];
	char *tz_name = getenv("TZ");
	
	int ch = 0;
	int starty, startx, maxy, maxx;
	int height = 5;
	int width = 30;
	
	getmaxyx(stdscr, maxy, maxx);
	
	starty = (maxy - height) / 2;
	startx = (maxx - width) / 2;
	
	data_dir_win = newwin(height, width, starty, startx);
	//only call derwin once
	data_dir_subwin = 
	derwin(data_dir_win, height - 2, width - 2, 0, 0);
	
	cbreak();
	keypad(data_dir_win, TRUE);
	clearok(data_dir_win, TRUE);
	wclear(data_dir_win);
	
	data_dir_field[0] = new_field(1, 25, 2, 2, 0, 0);
	set_field_back(data_dir_field[0], A_UNDERLINE);
	field_opts_off(data_dir_field[0], O_STATIC);
	field_opts_off(data_dir_field[0], O_AUTOSKIP);
	
	data_dir_field[1] = NULL;
	
	data_dir_form = new_form(data_dir_field);
	set_form_win(data_dir_form, data_dir_win);
	set_form_sub(data_dir_form, data_dir_subwin);
	
	touchwin(data_dir_win);
	post_form(data_dir_form);
	box(data_dir_win, 0, 0);
	mvwaddstr(data_dir_win, 1, 1, "filename?");
	wrefresh(data_dir_win);
	
	set_current_field(data_dir_form, data_dir_field[0]);
	wrefresh(data_dir_win);
	pos_form_cursor(data_dir_form);
	
	int done = 0;
	while(!done && (ch = wgetch(data_dir_win)))
	{
		mode = INSERT;
		switch (ch)
		{
			case '\n':
				form_driver(data_dir_form, REQ_VALIDATION);
				done = 1;
				break;
			default:
				form_driver(data_dir_form, ch);
				break;
		}
		wrefresh(data_dir_win);
	}
	
	char *filename = field_buffer(data_dir_field[0], 0);
	if (!filename || strlen(filename) == 0)
	{
		endwin();
		wprintw(data_dir_win, "ERR: file has no name");
		free(io->data_dir);
		free(io->filepath);
		free(io);
		return;
	}
	size_t i = strlen(filename);
	if (i >= 100)
	{
		endwin();
		wprintw(data_dir_win, "ERR: name too long");
		free(io->data_dir);
		free(io->filepath);
		free(io);
		return;
	}
	
	//trim filename
	while (i > 0 && filename[i - 1] == ' ')
		filename[--i] = '\0';

	char buffer[256] = {0};
	snprintf(buffer, 256, "%s%s%s",
	io->data_dir,
	io->filepath,
	filename
	);
	
	FILE *ifp = fopen(buffer, "w");
	if (!ifp)
	{
		endwin();
		perror("data_dir fopen");
		ERR_EXIT;
	}
			
	fprintf(ifp, "%d\n%d\n%d\n%d\n%d\n%s\n%f\n%f",
		cdata->tm_year,
		cdata->tm_mon,
		cdata->tm_mday,
		cdata->tm_hour,
		cdata->tm_min,
		tz_name,
		loc->dlat,
		loc->dlon
		);
		
		fclose(ifp);
		free(io->filepath);
		free(io->data_dir);
		free(io);
		unpost_form(data_dir_form);
		wclear(data_dir_win);
		touchwin(data_dir_win);
		wrefresh(data_dir_win);
		free_form(data_dir_form);
		
		for (size_t j = 0; j < 2; ++j)
			free_field(data_dir_field[j]);
		delwin(data_dir_subwin);
		delwin(data_dir_win);
}

void load_chart(FIELD *cdata_field[], Io *io)
{
	ITEM **load_files;
	MENU *load_menu;
	WINDOW *load_win;
	WINDOW *load_subwin;
	
}

void main_io(FIELD *cdata_field[], struct tm *cdata,
Location *loc, const char ch)
{
	struct passwd *pw = getpwuid(getuid());
	if (!pw) 
	{
		endwin();
		perror("petpwuid data");
		ERR_EXIT;
	}
	
	Io *io = calloc(1, sizeof(Io));
	if (!io)
	{
		endwin();
		perror("Io struct calloc");
		ERR_EXIT;
	}
	
	io->data_dir = 
	malloc(strlen(pw->pw_dir) + strlen("/.local/share") + 1);
	if (!io->data_dir)
	{
		endwin();
		perror("data_dir malloc");
		ERR_EXIT;
	}
	
	sprintf(io->data_dir, "%s/.local/share", pw->pw_dir);
	
	io->filepath = calloc(1, 256);
	if (!io->filepath)
	{
		endwin();
		perror("io filepath calloc");
		ERR_EXIT;
	}
	
	sprintf(io->filepath, "/astro/charts");

	if (ch == 'w')
		save_chart(cdata, loc, io);
	if (ch == 'e')
		load_chart(cdata_field, io);
}
