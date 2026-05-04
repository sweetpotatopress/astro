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
	struct stat buff;
	
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
	mvwaddstr(data_dir_win, 1, 1, "-o--filename?-o");
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
			case KEY_BACKSPACE:
				form_driver(data_dir_form, REQ_DEL_PREV);
				break;
			case KEY_LEFT:
				form_driver(data_dir_form, REQ_LEFT_CHAR);
				break;
			case KEY_RIGHT:
				form_driver(data_dir_form, REQ_RIGHT_CHAR);
				break;
			default:
				form_driver(data_dir_form, ch);
				break;
		}
		wrefresh(data_dir_win);
	}
	
	char *filename = field_buffer(data_dir_field[0], 0);
	if (!filename)
	{
		endwin();
		wprintw(data_dir_win, "ERR: file has no name");
		free(io->filepath);
		free(io);
		return;
	}
	
	// trimming fieldbuffer is unsafe, make copy
	// check for overwrite
	char copy[128] = {0};
	strncpy(copy, filename, sizeof(copy) - 1);
	size_t i = strlen(copy);
	
	while (i > 0 && copy[i-1] == ' ')
		copy[--i] = '\0';
		
	if (i >= 100)
	{
		endwin();
		wprintw(data_dir_win, "ERR: name too long");
		free(io->filepath);
		free(io);
		return;
	}
	
	char fn_buff[128];
	snprintf(fn_buff, 128, "%s%s",
	io->filepath,
	copy
	);
	
	if (stat(fn_buff, &buff) == 0)
	{
		wclear(data_dir_win);
		mvwprintw(data_dir_win, 1, 1,
		"overwrite:'%s'?\n -o--(y/n)-o:", copy);
		box(data_dir_win, 0, 0);
		wrefresh(data_dir_win);
		ch = getch();
		switch (ch)
		{
			case 'y':
				wclear(data_dir_win);
				mvwprintw(data_dir_win, 2, 1,
				"overwritten!--o-");
				box(data_dir_win, 0, 0);
				wrefresh(data_dir_win);
				getch();
				mode = NORMAL;
				break;
			case 'n':
				wclear(data_dir_win);
				mvwprintw(data_dir_win, 2, 1, 
				"your file is safe >w<");
				box(data_dir_win, 0, 0);
				wrefresh(data_dir_win);
				getch();
				endwin();
				free(io->filepath);
				free(io);
				mode = NORMAL;
				return;
			default:
				ch = getch();
		}
	}

	FILE *ifp = fopen(fn_buff, "w");
	if (!ifp)
	{
		endwin();
		perror("data_dir fopen");
		ERR_EXIT;
	}
			
	fprintf(ifp, "%d\n%d\n%d\n%d\n%d\n%s\n%f\n%f",
		cdata->tm_year,
		cdata->tm_mon + 1,
		cdata->tm_mday,
		cdata->tm_hour,
		cdata->tm_min,
		tz_name,
		loc->dlat,
		loc->dlon
		);
		
		fclose(ifp);
		free(io->filepath);
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
		
		mode = NORMAL;
}

void load_chart(Io *io)
{
	MENU *load_menu;
	WINDOW *load_win;
	WINDOW *load_subwin;
	DIR *chart_dir;
	struct dirent *entry;
	struct stat st;
	
	size_t i = 0;
	size_t file_count = 256;
	
	ITEM **load_files = calloc(file_count, sizeof(ITEM *));
	if (!load_files)
	{
		endwin();
		perror("load file calloc");
		ERR_EXIT;
	}
	
	char **strings = calloc(file_count, sizeof(char *));
	if (!strings)
	{
		endwin();
		perror("load_menu strings calloc");
		ERR_EXIT;
	}
	
	char fn_buff[1024] = {0};
	int max_width = 0;
	
	chart_dir = opendir(io->filepath);
	if (!chart_dir)
	{
		endwin();
		perror("load file opendir");
		ERR_EXIT;
	}
	
	while ((entry = readdir(chart_dir)) != NULL)
	{
		if (strcmp(entry->d_name, ".") != 0 &&
		strcmp(entry->d_name, "..") != 0)
		{
			snprintf(fn_buff, sizeof(fn_buff), "%s/%s",
			io->filepath,
			entry->d_name
			);
			stat(fn_buff, &st);
			
			strings[i] = malloc(1024);
			if (!strings[i])
			{
				endwin();
				perror("strings malloc");
				ERR_EXIT;
			}
			
			if (S_ISDIR(st.st_mode))
				snprintf(strings[i], 1024, "[%s]",
				entry->d_name);
			else
				snprintf(strings[i], 1024, " %s",
				entry->d_name);
				
			int len = (int)strlen(strings[i]) + 1;
			if (len > max_width)
				max_width = len;
				
			load_files[i] = new_item(strings[i], NULL);
			i++;
		}
	}
	load_files[i] = NULL;
	closedir(chart_dir);
	
	file_count = i;
	
	int width = max_width + 4;
	int height = (int)file_count + 3;
	
	if (width > COLS)
		width = COLS - 2;
	if (height > LINES)
		height = 18;
		
	int starty = (LINES - height) / 2;
	int startx = (COLS - width) / 2;
	
	load_win = newwin(height, width, starty, startx);
	if (!load_win)
	{
		endwin();
		perror("ERR: load_win");
		ERR_EXIT;
	}
	load_subwin = derwin(load_win, height - 2, width - 2, 1, 1);
	
	keypad(load_win, TRUE);
	clearok(load_win, TRUE);
	wclear(load_win);
	wrefresh(load_win);
	
	box(load_win, 0, 0);
	load_menu = new_menu(load_files);
	if (!load_menu)
	{
		endwin();
		perror("load menu");
		ERR_EXIT;
	}
	
	menu_opts_off(load_menu, O_NONCYCLIC);
	menu_opts_off(load_menu, O_SHOWDESC);
	set_menu_win(load_menu, load_win);
	set_menu_sub(load_menu, load_subwin);
	
	int iret = post_menu(load_menu);
	if (iret != E_OK)
	{
		endwin();
		perror("ERR: load_menu, post_menu");
		ERR_EXIT;
	}
	int menu_done = 0;
	int ch = 0;
	while (!menu_done)
	{
		ch = wgetch(load_win);
		switch(ch)
		{
			case 'j': case KEY_DOWN:
				menu_driver(load_menu, REQ_DOWN_ITEM);
				break;
			case 'q': 
				menu_done = 1;
				wclear(load_win);
				break;
		}
		wrefresh(load_win);
	}
	
	unpost_menu(load_menu);
	touchwin(load_win);
	wrefresh(load_win);
	free_menu(load_menu);
	for (size_t j = 0; j < file_count; ++j)
	{
		free_item(load_files[j]);
		free(strings[j]);
	}
	free(strings);
	free(load_files);
	free(io->filepath);
	free(io);
	
	wclear(load_win);
	delwin(load_subwin);
	delwin(load_win);
}

void main_io(struct tm *cdata,
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
	
	io->filepath = 
	malloc(strlen(pw->pw_dir) +
	strlen("/.local/share/astro/charts/") + 1);
	if (!io->filepath)
	{
		endwin();
		perror("data_dir malloc");
		ERR_EXIT;
	}
	
	snprintf(io->filepath, 1024,
	"%s/.local/share/astro/charts/", pw->pw_dir);

	if (ch == 'w')
		save_chart(cdata, loc, io);
	if (ch == 'e')
		load_chart(io);
}
