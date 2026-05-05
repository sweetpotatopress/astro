/* Copyright (C) 2026 yam lynn
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by the 
Free Software Foundation, either version 3 of the License, or (at your option)
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
			case 27:
				done = 1;
				endwin();
				return;
				break;
			default:
				form_driver(data_dir_form, ch);
				break;
		}
		wrefresh(data_dir_win);
		mode = NORMAL;
	}
	
	char *filename = field_buffer(data_dir_field[0], 0);
	
	// get field length, trim blank space from field_buffer, add null 0
	int len = 0;
	field_info(data_dir_field[0], NULL, NULL, NULL, &len, NULL, NULL);
	
	char *fn_copy = malloc((size_t)len + 1);
	if (!fn_copy)
	{
		endwin();
		perror("io fn_copy malloc");
		ERR_EXIT;
	}
	memcpy(fn_copy, filename, (size_t)len);
	
	--len;
	while(len >= 0 && fn_copy[len] == ' ')
		--len;
	if (len >= 0)
		fn_copy[len + 1] = '\0';
	
	if (len >= 100)
	{
		wprintw(data_dir_win, "ERR: name too long");
		wrefresh(data_dir_win);
		endwin();
		free(fn_copy);
		return;
	}
	if (len <= 0)
	{
		wprintw(data_dir_win, "ERR: name too short");
		wrefresh(data_dir_win);
		endwin();
		free(fn_copy);
		return;
	}
	
	// create file path 
	char fn_buff[256];
	snprintf(fn_buff, 256, "%s%s",
	io->filepath,
	filename
	);
	
	// if file exists with same name, ask to overwrite
	if (stat(fn_buff, &buff) == 0)
	{
		wclear(data_dir_win);
		mvwprintw(data_dir_win, 1, 1,
		"overwrite:'%s'?\n -o--(y/n)-o:", filename);
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

	// copy data to file, \n delimited
	fprintf(ifp, "%s\n%d\n%d\n%d\n%d\n%d\n%s\n%f\n%f",
		loc->city,
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
		free(fn_copy);
}

void load_chart(Io *io)
{
	DIR *chart_dir;
	struct dirent *entry;
	struct stat st;
	
	char *newpath = malloc(1024);
	if (!newpath)
	{
		endwin();
		perror("newpath malloc");
		ERR_EXIT;
	}
	
	char *homepath = malloc(strlen(io->filepath) + 1);
	if (!homepath)
	{
		endwin();
		perror("homepath malloc");
		ERR_EXIT;
	}
	memcpy(homepath, io->filepath, strlen(io->filepath) + 1);
	
	int load_done = 0;
	while (!load_done)
	{
		MENU *load_menu;
		WINDOW *load_win;
		WINDOW *load_subwin;
		
		size_t i = 0;
		size_t max_count = 20480;
		
		char fn_buff[1024] = {0};
		int max_width = 0;
		
		ITEM **load_files = calloc(max_count, sizeof(ITEM *));
		if (!load_files)
		{
			endwin();
			perror("load file calloc");
			ERR_EXIT;
		}

		char **i_name = calloc(max_count, sizeof(char *));
		if (!i_name)
		{
			endwin();
			perror("load_menu i_name calloc");
			ERR_EXIT;
		}
		
		char **i_desc = calloc(max_count, sizeof(char *));
		if (!i_desc)
		{
			endwin();
			perror("load_menu i_desc calloc");
			ERR_EXIT;
		}
	
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
				
				i_name[i] = malloc(sizeof(fn_buff));
				if (!i_name[i])
				{
					endwin();
					perror("i_name malloc");
					ERR_EXIT;
				}
				
				i_desc[i] = malloc(sizeof(fn_buff));
				if (!i_desc[i])
				{
					endwin();
					perror("i_desc malloc");
					ERR_EXIT;
				}
				
				snprintf(i_desc[i], sizeof(fn_buff), "%s",
				entry->d_name);
				
				if (S_ISDIR(st.st_mode))
					snprintf(i_name[i], sizeof(fn_buff), "[%s]",
					entry->d_name);
				else
					snprintf(i_name[i], sizeof(fn_buff), " %s",
					entry->d_name);
					
				// menu window width
				int len = (int)strlen(i_name[i]) + 1;
				if (len > max_width)
					max_width = len;
					
				load_files[i] = new_item(i_name[i], i_desc[i]);
				i++;
			}
		}
		load_files[i] = NULL;
		closedir(chart_dir);
		
		//to later free the appropriate amount of memory
		io->file_count = i;
		
		// window dimensions	
		int width = max_width + 4;
		int height = (int)io->file_count + 2;
		
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
		while (!menu_done && (ch = wgetch(load_win)))
		{
			switch(ch)
			{
				case 'j': case KEY_DOWN:
					menu_driver(load_menu, REQ_DOWN_ITEM);
					break;
				case 'k': case KEY_UP:
					menu_driver(load_menu, REQ_UP_ITEM);
					break;
				case 'l': case KEY_RIGHT:
					ITEM *cur = current_item(load_menu);
					const char *selected = item_description(cur);
					
					snprintf(newpath, 1024,
					"%s/%s", io->filepath, selected);
					
					//if file path is a directory
					if (stat(newpath, &st) == 0 &&
					S_ISDIR(st.st_mode))
					{
						free(io->filepath);
						
						io->filepath = malloc(strlen(newpath) + 1);
						if (!io->filepath)
						{
							endwin();
							perror("case l io->filepath");
							ERR_EXIT;
						}
						//copy new file path to open
						memcpy(io->filepath, newpath, strlen(newpath) + 1);
						
						wclear(load_win);
						menu_done = 1 ;
					}
					break;
				case 'h': case KEY_LEFT:
					free(io->filepath);
					
					io->filepath = malloc(strlen(homepath) + 1);
					if (!io->filepath)
					{
						endwin();
						perror("case h io->filepath");
						ERR_EXIT;
					}
					
					//return to homepath
					memcpy(io->filepath, homepath, strlen(homepath) + 1);
					
					wclear(load_win);
					menu_done = 1;
					break;
				case 'q': 
					load_done = 1;
					menu_done = 1;
					wclear(load_win);
					break;
				default:
					ch = wgetch(load_win);
			}
			wrefresh(load_win);
		}
		
		unpost_menu(load_menu);
		touchwin(load_win);
		wrefresh(load_win);
		free_menu(load_menu);
		for (size_t j = 0; j < io->file_count; ++j)
		{
			free_item(load_files[j]);
			free(i_name[j]);
			free(i_desc[j]);
		}
		free(i_name);
		free(i_desc);
		free(load_files);
		
		wclear(load_win);
		delwin(load_subwin);
		delwin(load_win);
	} // end of load_done loop
	free(newpath);
	free(homepath);
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
	
	io->filepath = malloc(1024);
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
		
	free(io->filepath);
	free(io);
}
