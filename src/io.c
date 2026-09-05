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

#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <ncurses.h>
#include <form.h>
#include <menu.h>
#include "swephexp.h"
#include "astro.h"
#include "io.h"

void xdg_check(char xdg_path[], const char *s)
{
	const char *home_dir = getenv("HOME");
	const char *xdg_data = getenv("XDG_DATA_HOME");
	const char *xdg_config = getenv("XDG_CONFIG_HOME");
	
	if (!home_dir || home_dir[0] == '\0')
		ERR_EXIT("$HOME not set");
	
	if (strcmp("config", s) == 0)
	{
		if (!xdg_config || xdg_config[0] == '\0')
			snprintf(xdg_path, MAXBUF,
			"%s/.config/astro/%s", home_dir, s);
		else
			snprintf(xdg_path, MAXBUF,
			"%s/astro/%s", xdg_config, s);
	}
	
	else if (strcmp("ephe", s) == 0 || strcmp("city-db", s) == 0 || strcmp("charts", s) == 0)
	{
		if (!xdg_data || xdg_data[0] == '\0')
			snprintf(xdg_path, MAXBUF,
			"%s/.local/share/astro/%s", home_dir, s);
		else
			snprintf(xdg_path, MAXBUF,
			"%s/astro/%s", xdg_data, s);
	}
	
	else
		ERR_EXIT("const char *s incorrect");
}

static size_t file_count(const char *path, const int r)
{
	DIR *dir;
	struct dirent *entry;
	size_t count = 0;
	
	if ((dir = opendir(path)) != NULL)
	{
		while ((entry = readdir(dir)) != NULL)
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
		{
			char filepath[MAXBUF];
			snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
			DIR *subdir = opendir(filepath);
			if (subdir && r)
			{
				closedir(subdir);
				count += file_count(filepath, 1);
			}
			else if (subdir)
				closedir(subdir);
			else
				count++;
		}
		closedir(dir);
	}
	return count;
}

static size_t name_to_item(struct io *io, ITEM **item, char **name, char **desc)
{
	struct dirent *entry;
	struct stat st;
	char fn_buf[MAXBUF] = {0};
	DIR *dir = opendir(io->filepath);
	if (!dir)
		ERR_EXIT("wahhhh");
	
	size_t count = 0;
	while ((entry = readdir(dir)) != NULL)
	{
		if (strcmp(entry->d_name, ".") != 0 &&
		strcmp(entry->d_name, "..") != 0)
		{
			snprintf(fn_buf, sizeof(fn_buf), "%s/%s", io->filepath, entry->d_name);
			
			name[count] = malloc(MAXBUF);
			if (!name[count])
				ERR_EXIT("S_ISDIR save_chart file_name");
				
			desc[count] = malloc(MAXBUF);
			if (!desc[count])
				ERR_EXIT("S_ISDIR save_chart file_desc");
		
			if (stat(fn_buf, &st) == -1)
				ERR_EXIT("no stat 4 u");
			snprintf(desc[count], sizeof(fn_buf), "%s", entry->d_name);
			if (S_ISDIR(st.st_mode))
				snprintf(name[count], sizeof(fn_buf), "[%s]", entry->d_name);
			else
				snprintf(name[count], sizeof(fn_buf), " %s", entry->d_name);
				
			item[count] = new_item(name[count], desc[count]);
			++count;
		}
	}
	item[count] = NULL;
	
	closedir(dir);
	return count;
}

static int print_save_menu(struct io *io, ITEM **item_save, char **name, char **desc, char *xdg_path)
{
	size_t icount = name_to_item(io, item_save, name, desc);
	
	int header = 4;
	int width = 25;
	int height = (int)icount + header;
	
	int starty = (LINES - height) / 2;
	int startx = (COLS - width) / 2;
	int save = 0;
	
	WINDOW *save_win = newwin(height, width, starty, startx);
	WINDOW *save_subwin = derwin(save_win, height - header, width - 2, 3, 1);
	MENU *save_menu = new_menu(item_save);
	
	box(save_win, 0, 0);
	keypad(save_win, TRUE);
	wbkgdset(save_win, COLOR_PAIR(M_COLOR));
	
	menu_opts_off(save_menu, O_NONCYCLIC);
	menu_opts_off(save_menu, O_SHOWDESC);
	set_menu_fore(save_menu, COLOR_PAIR(M_COLOR) | A_REVERSE);
	set_menu_back(save_menu, COLOR_PAIR(M_COLOR));
	set_menu_win(save_menu, save_win);
	set_menu_sub(save_menu, save_subwin);
	
	mvwprintw(save_win, 1, 1, " charts");
	mvwhline(save_win, 2, 1, ACS_HLINE, width - 2);
	
	post_menu(save_menu);
	wrefresh(save_win);
	
	struct stat st;
	const char *selected = NULL;
	ITEM *cur = NULL;
	char *mdir = NULL;
	char newpath[MAXPATH] = {0};
	char cur_dir[MAXBUF] = {0};
	snprintf(cur_dir, MAXBUF, "charts");
	
	int menu_done = 0, ch = 0;
	while (!menu_done && (ch = wgetch(save_win)))
	{
		cur = current_item(save_menu);
		selected = item_description(cur);
	
		switch(ch)
		{
			case 'j': case KEY_DOWN:
				menu_driver(save_menu, REQ_DOWN_ITEM);
				break;
			case 'k': case KEY_UP:
				menu_driver(save_menu, REQ_UP_ITEM);
				break;
			case '\n':
				snprintf(newpath, MAXPATH, "%s/%s/", io->filepath, selected);
				if (stat(newpath, &st) == 0 &&
				S_ISDIR(st.st_mode))
				{
					memcpy(io->filepath, newpath, strlen(newpath) + 1);
					snprintf(cur_dir, MAXBUF, "/%s", selected);
				}
				mvwprintw(save_win, 1, 1, "save to %s?", cur_dir);
				if ((ch = wgetch(save_win)) == '\n')
				{
					save = 1;
					menu_done = 1;
				}
				else
				{
					mvwhline(save_win, 1, 1, ' ',  width - 2);
					wrefresh(save_win);
					mvwprintw(save_win, 1, 1, "canceled save");
				}
				break;
			case 'l': case KEY_RIGHT:
				snprintf(newpath, MAXPATH, "%s/%s/", io->filepath, selected);
				if (stat(newpath, &st) == 0 &&
				S_ISDIR(st.st_mode))
				{
					memcpy(io->filepath, newpath, strlen(newpath) + 1);
					snprintf(cur_dir, MAXBUF, "/%s", selected);
					break;
				}
				break;
			case 'h': case KEY_LEFT:
				memcpy(io->filepath, xdg_path, strlen(xdg_path) + 1);
				snprintf(cur_dir, MAXBUF, "charts/");
			
				break;
			case 'm':
				mdir = calloc(1, 128);
				if (!mdir)
					ERR_EXIT("save_chart mdir case m");
				
				echo();
				mvwprintw(save_win, 1, 1, "dir name?:");
				mvwgetnstr(save_win, 1, 12, mdir, 127);
				noecho();
				
				snprintf(newpath, MAXPATH,
				"%s/%s/", io->filepath, mdir);
				
				if (mkdir(newpath, 0755) == -1)
					ERR_EXIT("save_menu mkdir fail");
				
				snprintf(cur_dir, MAXBUF, "%s/", mdir);
				
				free(mdir);
				break;
			case 'q': case 27:
				menu_done = 1;
				break;
		}
		if (ch == 'h' || ch == 'l' || ch == '\n' || ch == KEY_LEFT || ch == KEY_RIGHT || ch == 'm')
		{
			unpost_menu(save_menu);
			set_menu_items(save_menu, NULL); // ncurses doesnt free connected items
			for (size_t i = 0; i < icount; ++i)
			{
				free_item(item_save[i]);
				free(name[i]);
				free(desc[i]);
			}
			
			werase(save_win);
			wnoutrefresh(save_win);
			mvwprintw(save_win, 1, 1, " %s", cur_dir);
			mvwhline(save_win, 2, 1, ACS_HLINE, width - 2);
			
			icount = name_to_item(io, item_save, name, desc);
			set_menu_items(save_menu, item_save);
			
			width = 25;
			height = (int)icount + header;
	
			starty = (LINES - height) / 2;
			startx = (COLS - width) / 2;
		
			mvwin(save_win, starty, startx);
			mvwin(save_subwin, starty, startx);
			wresize(save_win, height, width);
			wresize(save_subwin, height - header, width -2);
		}
		box(save_win, 0, 0);
		post_menu(save_menu);
		doupdate();
	}
	
	unpost_menu(save_menu);
	free_menu(save_menu);
	for (size_t i = 0; i < icount; ++i)
	{
		free_item(item_save[i]);
		free(name[i]);
		free(desc[i]);
	}
	free(desc);
	free(name);
	free(item_save);
	
	werase(save_win);
	wrefresh(save_win);
	delwin(save_subwin);
	delwin(save_win);
	
	return save;
}

static void save_file_name(struct cdata *cdata, struct io *io)
{
	FIELD *save_field[2];
	struct stat buff;
	FORM *save_form;
	char *tz_name = getenv("TZ");
	char fn_buf[MAXBUF] = {0};
	
	int ch = 0;
	int starty, startx, maxy, maxx;
	int height = 5;
	int width = 30;
	
	getmaxyx(stdscr, maxy, maxx);
	
	starty = (maxy - height) / 2;
	startx = (maxx - width) / 2;
	
	WINDOW *save_win = newwin(height, width, starty, startx);
	WINDOW *save_subwin = derwin(save_win, height - 2, width - 2, 0, 0);
	
	wbkgdset(save_win, COLOR_PAIR(M_COLOR));
	
	cbreak();
	keypad(save_win, TRUE);
	
	save_field[0] = new_field(1, 25, 2, 2, 0, 0);
	set_field_back(save_field[0], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(save_field[0], O_STATIC);
	field_opts_off(save_field[0], O_AUTOSKIP);
	
	save_field[1] = NULL;
	
	save_form = new_form(save_field);
	set_form_win(save_form, save_win);
	set_form_sub(save_form, save_subwin);
	
	post_form(save_form);
	box(save_win, 0, 0);
	mvwaddstr(save_win, 1, 1, "-o--filename?-o");
	
	set_current_field(save_form, save_field[0]);
	wrefresh(save_win);
	pos_form_cursor(save_form);
	
	int done = 0;
	while(!done && (ch = wgetch(save_win)))
	{
		switch (ch)
		{
			case '\n':
				form_driver(save_form, REQ_VALIDATION);
				done = 1;
				break;
			case KEY_BACKSPACE:
				form_driver(save_form, REQ_DEL_PREV);
				break;
			case KEY_LEFT:
				form_driver(save_form, REQ_LEFT_CHAR);
				break;
			case KEY_RIGHT:
				form_driver(save_form, REQ_RIGHT_CHAR);
				break;
			case 27:
				done = 1;
				werase(save_subwin);
				wrefresh(save_subwin);
				werase(save_win);
				wrefresh(save_win);
				delwin(save_subwin);
				delwin(save_win);
				
				unpost_form(save_form);
				set_form_fields(save_form, NULL);
				for (int i = 0; i < 2; ++i)
					free_field(save_field[i]);
				free_form(save_form);
	
				return;
				break;
			default:
				form_driver(save_form, ch);
				break;
		}
		wrefresh(save_win);
	}
	
	char *filename = field_buffer(save_field[0], 0);
	
	// get field length, trim blank space from field_buffer, add null 0
	int len = 0;
	field_info(save_field[0], NULL, &len, NULL, NULL, NULL, NULL);
	
	char *fn_copy = malloc((size_t)len + 1);
	if (!fn_copy)
		ERR_EXIT("save_chart fn_copy malloc");
		
	memcpy(fn_copy, filename, (size_t)len +1);
	
	while (len > 0 && fn_copy[len - 1] == ' ')
		len--;
	fn_copy[len] = '\0';
	
	// create file path 
	snprintf(fn_buf, MAXBUF, "%s/%s",
	io->filepath,
	fn_copy
	);
	
	// if file exists with same name, ask to overwrite
	if (stat(fn_buf, &buff) == 0)
	{
		werase(save_win);
		mvwprintw(save_win, 1, 1,
		"overwrite:'%s'?\n -o--(y/n)-o:", filename);
		box(save_win, 0, 0);
		wrefresh(save_win);
		ch = getch();
		switch (ch)
		{
			case 'y':
				werase(save_win);
				mvwprintw(save_win, 2, 1,
				"overwritten!--o-");
				box(save_win, 0, 0);
				wrefresh(save_win);
				delwin(save_subwin);
				delwin(save_win);
				getch();
				break;
			case 'n':
				werase(save_win);
				mvwprintw(save_win, 2, 1, 
				"your file is safe >w<");
				box(save_win, 0, 0);
				wrefresh(save_win);
				delwin(save_subwin);
				delwin(save_win);
				getch();
				return;
			default:
				ch = getch();
		}
	}

	FILE *ifp = fopen(fn_buf, "w");
	if (!ifp)
		ERR_EXIT("ERR: save_chart ifp fopen");

	// copy data to file, \n delimited
	fprintf(ifp, "%s\n%s\n%s\n%d\n%d\n%d\n%d\n%d\n%d\n%s\n%f\n%f\n%d",
		cdata->city,
		cdata->state,
		cdata->country,
		cdata->tm_year,
		cdata->tm_mon,
		cdata->tm_mday,
		cdata->tm_hour,
		cdata->tm_min,
		cdata->tm_sec,
		tz_name,
		cdata->dlat,
		cdata->dlon,
		cdata->tm_isdst
		);
		
		fclose(ifp);
		unpost_form(save_form);
		werase(save_win);
		wrefresh(save_win);
		free_form(save_form);
		
		set_form_fields(save_form, NULL);
		for (size_t j = 0; j < 2; ++j)
			free_field(save_field[j]);
		delwin(save_subwin);
		delwin(save_win);
		
		free(fn_copy);
}

void save_chart(struct cdata *cdata, struct io *io, char xdg_path[])
{
	xdg_check(xdg_path, "charts");
	memcpy(io->filepath, xdg_path, strlen(xdg_path)+1);
	
	size_t size = file_count(xdg_path, 1) + 1;
	
	ITEM **item_save = calloc(size, sizeof(ITEM *));
	if (!item_save)
		ERR_EXIT("**item_save calloc");

	char **name = calloc(size, sizeof(char *));
	if (!name)
		ERR_EXIT("save_chart file_name calloc");
	
	char **desc = calloc(size, sizeof(char *));
	if (!desc)
		ERR_EXIT("save_chart file_desc calloc");
	
	if (print_save_menu(io, item_save, name, desc, xdg_path) == 1)
		save_file_name(cdata, io);
}

void load_chart(struct cdata *cdata, struct io *io, char xdg_path[])
{
	xdg_check(xdg_path, "charts");
	memcpy(io->filepath, xdg_path, strlen(xdg_path)+1);
	
	MENU *load_menu;
	WINDOW *load_win;
	WINDOW *load_subwin;
		
	struct dirent *entry;
	struct stat st;
	
	char *newpath = malloc(MAXPATH);
	if (!newpath)
		ERR_EXIT("load_chart newpath malloc");
	
	int load_done = 0;
	while (!load_done)
	{
		size_t i = 0;
		size_t cnt = file_count(xdg_path, 1);
		
		char fn_buf[MAXBUF] = {0};
		int max_width = 0;
		
		ITEM **item_load = calloc(cnt, sizeof(ITEM *));
		if (!item_load)
			ERR_EXIT("load_chart item_load calloc");

		char **file_name = calloc(cnt, sizeof(char *));
		if (!file_name)
			ERR_EXIT("load_chart file_name calloc");
		
		char **file_desc = calloc(cnt, sizeof(char *));
		if (!file_desc)
			ERR_EXIT("load_chart file_desc calloc");
	
		DIR *chart_dir = opendir(io->filepath);
		if (!chart_dir)
			ERR_EXIT("ERR: load_chart chart_dir");
		
		while ((entry = readdir(chart_dir)) != NULL)
		{
			// hide the up and down directory,
			//to restrict to only the charts dir
			
			if (strcmp(entry->d_name, ".") != 0 &&
			strcmp(entry->d_name, "..") != 0)
			{
				snprintf(fn_buf, sizeof(fn_buf), "%s/%s",
				io->filepath,
				entry->d_name
				);
				if (stat(fn_buf, &st) == -1)
					ERR_EXIT("load_chat stat");
				
				file_name[i] = malloc(sizeof(fn_buf));
				if (!file_name[i])
					ERR_EXIT("load_chart file_name[i] malloc");
				
				file_desc[i] = malloc(sizeof(fn_buf));
				if (!file_desc[i])
					ERR_EXIT("load_chart file_desc[i] malloc");
				
				snprintf(file_desc[i], sizeof(fn_buf), "%s",
				entry->d_name);
				
				if (S_ISDIR(st.st_mode))
					snprintf(file_name[i], sizeof(fn_buf), "[%s]",
					entry->d_name);
				else
					snprintf(file_name[i], sizeof(fn_buf), " %s",
					entry->d_name);
					
				// menu window width
				int len = (int)strlen(file_name[i]) + 1;
				if (len > max_width)
					max_width = len;
					
				item_load[i] = new_item(file_name[i], file_desc[i]);
				i++;
			}
		}
		if (i == 0)
		{
			item_load[0] = new_item("empty dir", " ");
			max_width = 10;
			i = 1;
		}
		
		item_load[i] = NULL;
		closedir(chart_dir);
		
		// window dimensions	
		int width = max_width + 4;
		if (max_width < 18)
			max_width = 18;
			
		int height = (int)i + 2;
		
		if (width > COLS)
			width = COLS - 2;
		if (height > LINES)
			height = 18;
			
		int starty = (LINES - height) / 2;
		int startx = (COLS - width) / 2;
		
		load_win = newwin(height, width, starty, startx);
		if (!load_win)
			ERR_EXIT("ERR: load_win newwin");
			
		load_subwin = derwin(load_win, height - 2, width - 2, 1, 1);
		
		wbkgdset(load_win, COLOR_PAIR(M_COLOR));
		
		keypad(load_win, TRUE);
		
		box(load_win, 0, 0);
		load_menu = new_menu(item_load);
		if (!load_menu)
			ERR_EXIT("ERR: load_menu new_menu");
		
		set_menu_win(load_menu, load_win);
		set_menu_sub(load_menu, load_subwin);
		set_menu_back(load_menu, COLOR_PAIR(M_COLOR));
		set_menu_fore(load_menu, COLOR_PAIR(M_COLOR) | A_REVERSE);
		menu_opts_off(load_menu, O_NONCYCLIC);
		menu_opts_off(load_menu, O_SHOWDESC);
		
		int iret = post_menu(load_menu);
		if (iret != E_OK)
			ERR_EXIT("ERR: post_menu(load_menu)");
		
		ITEM *cur = NULL;
		const char *selected = NULL;
		char *buffer = NULL;
		
		FILE *fp;
		char field[FMAX][562] = {0};
		
		char *endptr = NULL;
		long lret;
		double dret;
		errno = 0;
		
		int count = 0;
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
				case 'l': case KEY_RIGHT: case '\n':
					cur = current_item(load_menu);
					selected = item_description(cur);
					
					snprintf(io->filename, MAXBUF, "%s", selected);
					
					snprintf(newpath, MAXPATH,
					"%s/%s", io->filepath, selected);
			
					// if file path is a directory
					if (stat(newpath, &st) == 0 &&
					S_ISDIR(st.st_mode))
					{
						// copy new file path to open
						memcpy(io->filepath, newpath, strlen(newpath) + 1);
						
						werase(load_win);
						menu_done = 1 ;
						break;
					}
					
					// load selected file
					
					buffer = malloc(MAXBUF);
					if (!buffer)
						ERR_EXIT("load_chart case l buffer");
					
					fp = fopen(newpath, "r");
					if (fp == NULL)
						ERR_EXIT("load_chart fopen fail");
					
					while (fgets(buffer, MAXBUF, fp) != NULL && count < FMAX)
					{
						buffer[strcspn(buffer, "\n")] = 0;
						memcpy(field[count++], buffer, strlen(buffer) + 1);
					}
						
					memcpy(cdata->city, field[FCITY], strlen(field[FCITY]) + 1);
					memcpy(cdata->state, field[FSTATE], strlen(field[FSTATE]) + 1);
					memcpy(cdata->country, field[FCOUNTRY], strlen(field[FCOUNTRY]) + 1);
								
					lret = strtol(field[FYEAR], &endptr, 10);
					if (errno != ERANGE)
						cdata->tm_year = (int)lret;
					else
						cdata->tm_year = 1970;
								
					lret = strtol(field[FMONTH], &endptr, 10);
					if (errno != ERANGE && lret != -1)
						cdata->tm_mon = (int)lret;
					else
						cdata->tm_mon = 1;
								
					lret = strtol(field[FDAY], &endptr, 10);
					if (errno != ERANGE && lret != -1)
						cdata->tm_mday = (int)lret;
					else
						cdata->tm_mday = 1;
								
					lret = strtol(field[FHOUR], &endptr, 10);
					if (errno != ERANGE && lret != -1) 
						cdata->tm_hour = (int)lret;
					else
						cdata->tm_hour = 1;
									
					lret = strtol(field[FMIN], &endptr, 10);
					if (errno != ERANGE && lret != -1)
						cdata->tm_min = (int)lret;
					else
						cdata->tm_min = 1;
						
					lret = strtol(field[FSEC], &endptr, 10);
					if (errno != ERANGE && lret != -1)
						cdata->tm_sec = (int)lret;
					else
						cdata->tm_sec = 0;
								
					if (setenv("TZ", field[FTZ], 1) != 0)
						ERR_EXIT("ERR: TZ setenv fail field_to_member");
					tzset();
							
					dret = strtod(field[FLAT], &endptr);
					if (errno != ERANGE)
						cdata->dlat = dret;
					else
						cdata->dlat = 0.0;
							
					dret = strtod(field[FLON], &endptr);
					if (errno != ERANGE)
						cdata->dlon = dret;
					else
						cdata->dlon = 0.0;
					
					lret = strtol(field[FDST], &endptr, 10);
					if (lret > 0)
						cdata->tm_isdst = YDST;
					else if (lret == 0)
						cdata->tm_isdst = NDST;
						
					free(buffer);
					fclose(fp);
					
					load_done = 1;
					menu_done = 1;
					break;
				case 'h': case KEY_LEFT:
					//return to homepath
					memcpy(io->filepath, xdg_path, strlen(xdg_path) + 1);
					
					werase(load_win);
					menu_done = 1;
					break;
				case 'q': case 27:
					load_done = 1;
					menu_done = 1;
					werase(load_win);
					break;
				default:
					ch = wgetch(load_win);
			}
			wrefresh(load_win);
		}
		
		unpost_menu(load_menu);
		free_menu(load_menu);
		for (size_t j = 0; j < cnt; ++j)
		{
			free_item(item_load[j]);
			free(file_name[j]);
			free(file_desc[j]);
		}
		free(file_name);
		free(file_desc);
		free(item_load);
		
		werase(load_win);
		wrefresh(load_win);
		delwin(load_subwin);
		delwin(load_win);
	} // end of load_done loop
	free(newpath);
}

