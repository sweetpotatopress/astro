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
#include "chronos.h"
#include "ui.h"

static void path_env(char *env, const char *xdg,  const char *f)
{
	const char *dir = getenv(xdg);
	if (dir && dir[0] != '\0')
	{
		snprintf(env, MAXBUF, "%s", dir);
		return;
	}
	const char *home = getenv("HOME");
	if (!home || home[0] == '\0')
		ERR_EXIT("ERR: $HOME not set");
	snprintf(env, MAXBUF, "%s/%s", home, f);
}

void path_check(char *path, const char *s)
{
	memset(path, 0, MAXPATH);
	char env[MAXBUF] = {0};

	if (strcmp("config_home", s) == 0)
	{
		path_env(env, "XDG_CONFIG_HOME", ".config");
		snprintf(path, MAXPATH, "%s/%s", env, "astro");
	}
	else if (strcmp("config", s) == 0)
	{
		path_env(env, "XDG_CONFIG_HOME", ".config");
		snprintf(path, MAXPATH, "%s/%s", env, "astro/config");
	}
	else if (strcmp("data_home", s) == 0)
	{
		path_env(env, "DDG_DATA_HOME", ".local/share");
		snprintf(path, MAXPATH, "%s/%s", env, "astro");
	}
	else if (strcmp("charts", s) == 0)
	{
		path_env(env, "XDG_DATA_HOME", ".local/share");
		snprintf(path, MAXPATH, "%s/%s", env, "astro/charts");
	}
	else if (strcmp("diary", s) == 0)
	{
		path_env(env, "XDG_DATA_HOME", ".local/share");
		snprintf(path, MAXPATH, "%s/%s", env, "astro/diary");
	}
	else if (strcmp("ephe", s) == 0)
	{
		snprintf(path, MAXPATH, "%s/%s", ASTRO_DATA_DIR, "ephe");
		if (strlen(path) > 255)
			ERR_EXIT("ERR: ephe path too long");
	}
	else if (strcmp("city-db", s) == 0)
		snprintf(path, MAXPATH, "%s/%s", ASTRO_DATA_DIR, "city-db");
	else
		ERR_EXIT("path_check string error");
}

void datadir_check(void)
{
	char path[MAXPATH] = {0};
	
	path_check(path, "config_home");
	if (access(path, F_OK) == -1)
		if (mkdir(path, 0755) == -1)
			ERR_EXIT("ERR: config_home mkdir");
			
	path_check(path, "data_home");
	if (access(path, F_OK) == -1)
		if (mkdir(path, 0755) == -1)
			ERR_EXIT("ERR: data_home mkdir");
	
	path_check(path, "charts");
	if (access(path, F_OK) == -1)
		if (mkdir(path, 0755) == -1)
			ERR_EXIT("ERR: charts mkdir");
			
	path_check(path, "diary");
	if (access(path, F_OK) == -1)
		if (mkdir(path, 0755) == -1)
			ERR_EXIT("ERR: diary mkdir");
	
	path_check(path, "ephe");
	if (access(path, F_OK) == -1)
		ERR_EXIT("ERR: no ephe installed");
	path_check(path, "city-db");
	if (access(path, F_OK) == -1)
		ERR_EXIT("ERR: no city-db installed");
}

size_t file_count(const char *path, const int r)
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

size_t name_to_item(char *filepath, ITEM **item, char **name, char **desc)
{
	struct dirent *entry;
	struct stat st;
	char buf[MAXBUF] = {0};
	DIR *dir = opendir(filepath);
	if (!dir)
		ERR_EXIT("name_to_item opendir failure");
	
	size_t count = 0;
	while ((entry = readdir(dir)) != NULL)
	{
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
		{
			snprintf(buf, sizeof(buf), "%s/%s", filepath, entry->d_name);
			
			name[count] = ecalloc(1,MAXBUF);
			desc[count] = ecalloc(1,MAXBUF);
		
			if (stat(buf, &st) == -1)
				ERR_EXIT("no stat 4 u");
			snprintf(desc[count], sizeof(buf), "%s", entry->d_name);
			if (S_ISDIR(st.st_mode))
				snprintf(name[count], sizeof(buf), "[%s]", entry->d_name);
			else
				snprintf(name[count], sizeof(buf), " %s", entry->d_name);
				
			item[count] = new_item(name[count], desc[count]);
			++count;
		}
	}
	item[count] = NULL;
	
	closedir(dir);
	return count;
}

void back_dir(char *filepath, char *cur_dir)
{
	char *last = strrchr(filepath, '/');
	char name[128] = {0};
	if(last)
		*last = '\0';
	last = strrchr(filepath, '/');
		if(last)
	memcpy(name, last, strlen(last) + 1);
	snprintf(cur_dir, MAXBUF, " %s", name);
}

static void rebuild_menu(struct ui *ui, char *path, char ls)
{
	unpost_menu(ui->fb_menu);
	set_menu_items(ui->fb_menu, NULL);
	for (size_t i = 0; i < ui->fb_c; ++i)
	{
		free_item(ui->fb_item[i]);
		free(ui->fb_name[i]);
		free(ui->fb_desc[i]);
	}
			
	werase(ui->fb_win);
	wnoutrefresh(ui->fb_win);
	mvwprintw(ui->fb_win, 1, 1, "%s", ui->fb_cur_dir);
	mvwprintw(ui->fb_win, 1, ui->fb_width - 4, "{%c}", ls);
	mvwhline(ui->fb_win, 2, 1, ACS_HLINE, ui->fb_width - 2);
			
	ui->fb_c = name_to_item(path, ui->fb_item, ui->fb_name, ui->fb_desc);
	set_menu_items(ui->fb_menu, ui->fb_item);
			
	ui->fb_width = 25;
	ui->fb_height = (int)ui->fb_c + ui->fb_header;
	if (ui->fb_c > 16)
		ui->fb_height = 16 + ui->fb_header;
			
	ui->fb_sy = (LINES - ui->fb_height) / 2;
	ui->fb_sx = (COLS - ui->fb_width) / 2;
	
	wresize(ui->fb_win, ui->fb_height, ui->fb_width);
	wresize(ui->fb_subwin, ui->fb_height - ui->fb_header, ui->fb_width -2);
	mvwin(ui->fb_win, ui->fb_sy, ui->fb_sx);
	mvwin(ui->fb_subwin, ui->fb_sy, ui->fb_sx);
}

static int save_options(struct ui *ui, char *path, char *head, char ls)
{
	struct stat st;
	const char *selected = NULL;
	ITEM *cur = NULL;
	snprintf(ui->fb_cur_dir, MAXBUF, " %s", head);
	
	char *mdir = NULL;
	int menu_done = 0, ch = 0;
	while (!menu_done && (ch = wgetch(ui->fb_win)))
	{
		cur = current_item(ui->fb_menu);
		selected = item_description(cur);
	
		switch(ch)
		{
			case 'j': case KEY_DOWN:
				menu_driver(ui->fb_menu, REQ_DOWN_ITEM);
				break;
			case 'k': case KEY_UP:
				menu_driver(ui->fb_menu, REQ_UP_ITEM);
				break;
			case '\n':
				snprintf(ui->fb_newpath, MAXPATH, "%s/%s", path, selected);
				if (stat(ui->fb_newpath, &st) == 0 && S_ISDIR(st.st_mode))
				{
					memcpy(path, ui->fb_newpath, strlen(ui->fb_newpath) + 1);
					snprintf(ui->fb_cur_dir, MAXBUF, "/%s", selected);
				}
				mvwprintw(ui->fb_win, 1, 1, "save to %s?", ui->fb_cur_dir);
				if ((ch = wgetch(ui->fb_win)) == '\n')
				{
					ui->fb_save = 1;
					menu_done = 1;
				}
				else
				{
					mvwhline(ui->fb_win, 1, 1, ' ',  ui->fb_width - 2);
					wrefresh(ui->fb_win);
					mvwprintw(ui->fb_win, 1, 1, "canceled save");
				}
				break;
			case 'l': case KEY_RIGHT:
				snprintf(ui->fb_newpath, MAXPATH, "%s/%s", path, selected);
				if (stat(ui->fb_newpath, &st) == 0 && S_ISDIR(st.st_mode))
				{
					memcpy(path, ui->fb_newpath, strlen(ui->fb_newpath) + 1);
					snprintf(ui->fb_cur_dir, MAXBUF, " /%s", selected);
					break;
				}
				break;
			case 'h': case KEY_LEFT: 
				if (strcmp(path, ui->fb_base_dir) == 0)
					break;
				back_dir(path, ui->fb_cur_dir);
				break;
			case 'm':
				mdir = ecalloc(1, 128);
				
				echo();
				mvwprintw(ui->fb_win, 1, 1, "dir name?:");
				mvwgetnstr(ui->fb_win, 1, 12, mdir, 127);
				noecho();
				
				snprintf(ui->fb_newpath, MAXPATH, "%s/%s/", path, mdir);
				
				if (mkdir(ui->fb_newpath, 0755) == -1)
					ERR_EXIT("menu mkdir fail");
				
				snprintf(ui->fb_cur_dir, MAXBUF, "%s/", mdir);
				
				free(mdir);
				break;
			case 'q': case 27:
				menu_done = 1;
				break;
		}
		if (ch == 'h' || ch == 'l' || ch == '\n' || ch == KEY_LEFT || ch == KEY_RIGHT || ch == 'm')
			rebuild_menu(ui, path, ls);
			
		box(ui->fb_win, 0, 0);
		post_menu(ui->fb_menu);
		doupdate();
	}
	return ui->fb_save;
}

static void save_file_name(struct cdata *cdata, char *path)
{
	struct stat buff;
	
	int height = 5;
	int width = 30;
	
	int starty = (LINES- height) / 2;
	int startx = (COLS - width) / 2;
	
	WINDOW *win = newwin(height, width, starty, startx);
	WINDOW *subwin = derwin(win, height - 2, width - 2, 0, 0);
	
	wbkgdset(win, COLOR_PAIR(M_COLOR));
	
	keypad(win, TRUE);
	
	FIELD *save_field[2];
	save_field[0] = new_field(1, 25, 2, 2, 0, 0);
	set_field_back(save_field[0], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(save_field[0], O_STATIC);
	field_opts_off(save_field[0], O_AUTOSKIP);
	
	save_field[1] = NULL;
	
	FORM *save_form = new_form(save_field);
	set_form_win(save_form, win);
	set_form_sub(save_form, subwin);
	
	post_form(save_form);
	box(win, 0, 0);
	mvwaddstr(win, 1, 1, "-o--filename?-o");
	
	set_current_field(save_form, save_field[0]);
	wrefresh(win);
	pos_form_cursor(save_form);
	
	int done = 0, ch = 0;
	while(!done && (ch = wgetch(win)))
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
				werase(subwin);
				wrefresh(subwin);
				werase(win);
				wrefresh(win);
				delwin(subwin);
				delwin(win);
				
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
		wrefresh(win);
	}
	
	char *filename = field_buffer(save_field[0], 0);
	char fn[MAXBUF] = {0};
	char newpath[MAXPATH] = {0};
	int len = 0;
	
	field_info(save_field[0], NULL, &len, NULL, NULL, NULL, NULL);
	
	snprintf(fn, MAXBUF, "%s", filename);
	
	while (len > 0 && fn[len - 1] == ' ')
		len--;
	fn[len] = '\0';
	
	snprintf(newpath, MAXPATH, "%s/%s", path, fn);
	
	if (stat(newpath, &buff) == 0)
	{
		werase(win);
		mvwprintw(win, 1, 1,
		"overwrite:'%s'?\n -o--(y/n)-o:", filename);
		box(win, 0, 0);
		wrefresh(win);
		ch = getch();
		switch (ch)
		{
			case 'y':
				werase(win);
				mvwprintw(win, 2, 1, "overwritten!--o-");
				box(win, 0, 0);
				wrefresh(win);
				delwin(subwin);
				delwin(win);
				getch();
				break;
			case 'n':
				werase(win);
				mvwprintw(win, 2, 1, "your file is safe >w<");
				box(win, 0, 0);
				wrefresh(win);
				delwin(subwin);
				delwin(win);
				getch();
				return;
			default:
				ch = getch();
		}
	}
	
	if (strlen(cdata->city) < 2)
		strncpy(cdata->city, "0", MAXBUF);
	if (strlen(cdata->state) < 2)
		strncpy(cdata->state, "0", MAXBUF);
	if (strlen(cdata->country) < 2)
		strncpy(cdata->country, "0", MAXBUF);

	FILE *ifp = fopen(newpath, "w");
	if (!ifp)
		ERR_EXIT("ERR: save_chart ifp fopen");

	fprintf(ifp, "%s,%s,%s,%d,%d,%d,%d,%d,%d,%s,%f,%f",
		cdata->city,
		cdata->state,
		cdata->country,
		cdata->year,
		cdata->mon,
		cdata->mday,
		cdata->hour,
		cdata->min,
		cdata->sec,
		cdata->timezone,
		cdata->dlat,
		cdata->dlon);
		
		fclose(ifp);
		unpost_form(save_form);
		werase(win);
		wrefresh(win);
		
		set_form_fields(save_form, NULL);
		for (size_t j = 0; j < 2; ++j)
			free_field(save_field[j]);
		free_form(save_form);
		
		delwin(subwin);
		delwin(win);
}

static void converttocomma(char *path)
{ // temporary function to convert legacy \n delim'd chart files
	FILE *fp = fopen(path, "r");
	if (fp == NULL)
		ERR_EXIT("converttocomma fopen");
			
	int ch;
	char buf[MAXBUF] = {0};
	int i = 0;
	while ((ch = fgetc(fp)) != EOF)
		buf[i++] = ch;
	buf[i] = '\0';
	
	for (size_t j = 0; j < strlen(buf); j++)
	{
		if (j > 0 && buf[j-1] == '\n' && buf[j] == '\n')
		{
			buf[j] = 'E';
			for (size_t c = strlen(buf)+1; c > j; c--)
				buf[c] = buf[c-1];
			buf[j+1] = ',';
		}
	}
	for (size_t j = 0; j < strlen(buf); j++)
	{
		if (buf[j] == '\n')
			buf[j] = ',';
	}
	
	fclose(fp);
	
	fp = fopen(path, "w");
	if (!fp)
		ERR_EXIT("convertocomma fopen");
	fprintf(fp, "%s", buf);
	fclose(fp);
}

void load_options(struct cdata *cdata, struct ui *ui, char *path, char *head, char ls)
{
	struct stat st;
	const char *selected = NULL;
	ITEM *cur = NULL;
	
	char buffer[MAXBUF] = {0};
	char field[FMAX][MAXBUF] = {0};
	snprintf(ui->fb_cur_dir, MAXBUF, " %s", head);
	
	char *endptr = NULL;
	long lret;
	double dret;
	errno = 0;
	
	int ch = 0, count = 0, menu_done = 0;
	while (!menu_done && (ch = wgetch(ui->fb_win)))
	{
		cur = current_item(ui->fb_menu);
		selected = item_description(cur);
		
		switch(ch)
		{
			case 'j': case KEY_DOWN:
				menu_driver(ui->fb_menu, REQ_DOWN_ITEM);
				break;
			case 'k': case KEY_UP:
				menu_driver(ui->fb_menu, REQ_UP_ITEM);
				break;
			case 'l': case KEY_RIGHT: case '\n':
				if ((int)ui->fb_c == 0)
					break;
				
				int n = snprintf(ui->fb_newpath, MAXPATH, "%s/%s", path, selected);
				if (n >= MAXPATH)
					ERR_EXIT("print_menu newpath too long");
		
				if (stat(ui->fb_newpath, &st) == 0 && S_ISDIR(st.st_mode))
				{
					memcpy(path, ui->fb_newpath, strlen(ui->fb_newpath) + 1);
					snprintf(ui->fb_cur_dir, MAXBUF, " /%s", selected);
					break;
				}
				
				snprintf(cdata->chart_name, MAXBUF, "%s", selected);
				
				if (strcmp(head, "/charts") == 0)
					converttocomma(ui->fb_newpath);
				
				FILE *fp = fopen(ui->fb_newpath, "r");
				if (fp == NULL)
					ERR_EXIT("print_menu newpath fopen");
				
				if (fgets(buffer, MAXBUF, fp) != NULL)
				{
					char *it = xstrtok(buffer, ",");
				
					while(it)
					{
						snprintf(field[count++], MAXBUF, "%s", it);
						it = xstrtok(NULL, ",");
					}
				}
				
				memcpy(cdata->city, field[FCITY], strlen(field[FCITY]) + 1);
				memcpy(cdata->state, field[FSTATE], strlen(field[FSTATE]) + 1);
				memcpy(cdata->country, field[FCOUNTRY], strlen(field[FCOUNTRY]) + 1);
					
				lret = strtol(field[FYEAR], &endptr, 10);
				if (errno != ERANGE)
					cdata->year = (int)lret;
				else
					cdata->year = 1970;
				errno = 0;
							
				lret = strtol(field[FMONTH], &endptr, 10);
				if (errno != ERANGE && lret != -1)
					cdata->mon = (int)lret;
				else
					cdata->mon = 1;
				errno = 0;
							
				lret = strtol(field[FDAY], &endptr, 10);
				if (errno != ERANGE && lret != -1)
					cdata->mday = (int)lret;
				else
					cdata->mday = 1;
				errno = 0;
							
				lret = strtol(field[FHOUR], &endptr, 10);
				if (errno != ERANGE && lret != -1) 
					cdata->hour = (int)lret;
				else
					cdata->hour = 1;
				errno = 0;
								
				lret = strtol(field[FMIN], &endptr, 10);
				if (errno != ERANGE && lret != -1)
					cdata->min = (int)lret;
				else
					cdata->min = 1;
				errno = 0;
					
				lret = strtol(field[FSEC], &endptr, 10);
				if (errno != ERANGE && lret != -1)
					cdata->sec = (int)lret;
				else
					cdata->sec = 0;
				errno = 0;
							
				if (setenv("TZ", field[FTZ], 1) != 0)
					ERR_EXIT("ERR: print_menu TZ setenv");
				tzset();
				memcpy(cdata->timezone, field[FTZ], strlen(field[FTZ]) + 1);
						
				dret = strtod(field[FLAT], &endptr);
				if (errno != ERANGE)
					cdata->dlat = dret;
				else
					cdata->dlat = 0.0;
				errno = 0;
						
				dret = strtod(field[FLON], &endptr);
				if (errno != ERANGE)
					cdata->dlon = dret;
				else
					cdata->dlon = 0.0;
				errno = 0;
				
				fclose(fp);
				
				menu_done = 1;
				break;
			case 'h': case KEY_LEFT:
				if (strcmp(path, ui->fb_base_dir) == 0)
					break;
				back_dir(path, ui->fb_cur_dir);
				break;
			case 'q': case 27:
				menu_done = 1;
				ui->fb_cancel = TRUE;
				break;
		}
		if (ch == 'h' || ch == 'l' || ch == '\n' || ch == KEY_LEFT || ch == KEY_RIGHT || ch == 'm')
			rebuild_menu(ui, path, ls);
			
		box(ui->fb_win, 0, 0);
		post_menu(ui->fb_menu);
		doupdate();
	}
}

void print_menu(struct cdata *cdata, struct ui *ui, char *path, char *head, char ls, bool sl)
{
	ui->fb_save = 0;
	ui->fb_c = file_count(path, 1) + 1;
	
	ui->fb_item = ecalloc(ui->fb_c, sizeof(ITEM *));
	ui->fb_name = ecalloc(ui->fb_c, sizeof(char *));
	ui->fb_desc = ecalloc(ui->fb_c, sizeof(char *));
	
	ui->fb_base_dir = ecalloc(1, MAXPATH);
	memcpy(ui->fb_base_dir, path, MAXPATH);
	
	ui->fb_c = name_to_item(path, ui->fb_item, ui->fb_name, ui->fb_desc);
	
	ui->fb_header = 4;
	ui->fb_width = 25;
	ui->fb_height = (int)ui->fb_c + ui->fb_header;
	if (ui->fb_c > 16)
		ui->fb_height = 16 + ui->fb_header;
	
	ui->fb_sy = (LINES - ui->fb_height) / 2;
	ui->fb_sx = (COLS - ui->fb_width) / 2;
	ui->fb_save = 0;
	
	ui->fb_win = newwin(ui->fb_height, ui->fb_width, ui->fb_sy, ui->fb_sx);
	ui->fb_subwin = derwin(ui->fb_win, ui->fb_height - ui->fb_header, ui->fb_width - 2, 3, 1);
	ui->fb_menu = new_menu(ui->fb_item);
	
	box(ui->fb_win, 0, 0);
	keypad(ui->fb_win, TRUE);
	wbkgdset(ui->fb_win, COLOR_PAIR(M_COLOR));
	
	menu_opts_off(ui->fb_menu, O_NONCYCLIC);
	menu_opts_off(ui->fb_menu, O_SHOWDESC);
	set_menu_fore(ui->fb_menu, COLOR_PAIR(M_COLOR) | A_REVERSE);
	set_menu_back(ui->fb_menu, COLOR_PAIR(M_COLOR));
	set_menu_win(ui->fb_menu, ui->fb_win);
	set_menu_sub(ui->fb_menu, ui->fb_subwin);
	
	mvwprintw(ui->fb_win, 1, 1, " %s", head);
	mvwprintw(ui->fb_win, 1, ui->fb_width - 4, "{%c}", ls);
	mvwhline(ui->fb_win, 2, 1, ACS_HLINE, ui->fb_width - 2);
	
	post_menu(ui->fb_menu);
	wrefresh(ui->fb_win);
	
	if (sl == SAVE)
	{
		if (save_options(ui, path, head, ls) == 1)
			save_file_name(cdata, path);
	}
	else
	{
		load_options(cdata, ui, path, head, ls);
		memcpy(path, ui->fb_newpath, strlen(ui->fb_newpath) + 1);
	}
	
	unpost_menu(ui->fb_menu);
	free_menu(ui->fb_menu);
	for (size_t i = 0; i < ui->fb_c; ++i)
	{
		free_item(ui->fb_item[i]);
		free(ui->fb_name[i]);
		free(ui->fb_desc[i]);
	}
	free(ui->fb_desc);
	free(ui->fb_name);
	free(ui->fb_item);
	free(ui->fb_base_dir);
	
	werase(ui->fb_win);
	wrefresh(ui->fb_win);
	delwin(ui->fb_subwin);
	delwin(ui->fb_win);
}

void save_chart(struct cdata *cdata, struct ui *ui)
{
	char path[MAXPATH] = {0};
	path_check(path, "charts");
	
	print_menu(cdata, ui, path, "/charts", 's', SAVE);
}

void load_chart(struct cdata *cdata, struct ui *ui)
{
	char path[MAXPATH] = {0};
	path_check(path, "charts");
	
	print_menu(cdata, ui, path, "/charts", 'l', LOAD);
}
