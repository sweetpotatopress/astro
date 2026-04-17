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

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
 
int city_search(FILE *ifp, char *search, Location **choices, int max_choices)
{
	int i = 0;
	char line[1024] = {0};
	
	while (fgets(line, sizeof(line), ifp) != NULL)
	{
		int len = strlen(line);
		if (line[len - 1] == '\n')
			line[len - 1] = '\0';
		
		char *copy = malloc(strlen(line) + 1);
		strcpy(copy, line);
		
		char *token = strtok(copy, "\t");
		int field_count = 0;
		char *fields[19] = {0};
				
		while (token != NULL && field_count < 19)
		{
			fields[field_count] = malloc(strlen(token) + 1);
			strcpy(fields[field_count], token);
			field_count++;
			token = strtok(NULL, "\t");
		}
		
		if (field_count > 1 && strstr(fields[1], search) != NULL)
		{
			Location *location = malloc(sizeof(Location));
			location->city = fields[1];
			location->country = fields[8];
			location->latitude = fields[4];
			location->longitude = fields[5];
			choices[i++] = location;
		}
		else
		{
			for(int j = 0; j < field_count; j++)
				free(fields[j]);
		}
		free(copy);
	}
	if (i >= max_choices)
		return -1;
	else
		return i;
}

void location_to_string(Location *loc, char *buffer, int buffer_size)
{
	snprintf(buffer, buffer_size, "%s\t%s\t%s\t%s",
		loc->city,
		loc->country,
		loc->latitude,
		loc->longitude);
}

void print_menu(WINDOW *menu_win, int highlight, Location **choices)
{ 
	int x, y, i;  
	int menu = 1;
	x = y = 3;
	char buffer[256];
	
    box(menu_win, 0, 0); 
    mvwprintw(menu_win, menu, menu, "%s", " city\t\tcountry\tlatitude longitude");
    for(i = 0; i < n_choices ; ++i)
    {
    	location_to_string(choices[i], buffer, sizeof(buffer));
    	
    	if (highlight == i + 1) 
   		{    
  			wattron(menu_win, A_REVERSE); 
 			mvwprintw(menu_win, y, x, "%s", buffer);
			wattroff(menu_win, A_REVERSE);
		}
		else 
			mvwprintw(menu_win, y, x, "%s", buffer); 
		++y; 
	} 
	wrefresh(menu_win); 
} 

int main_search(char *argv)
{
	FILE *fp;
	const char *path = "cities";
	char *search = argv;
	int choice = 0;
	int max_loc = 100;
	Location **choices = malloc(sizeof(Location *) * max_loc);
	//ncurses
	int highlight = 1;
	WINDOW *menu_win;
	static int startx, starty, width, height;
	int c;
	
	
	fp = fopen(path, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "can't open %s\n", path);
		free(choices);
		getch();
		endwin();
		exit(1);
	}

	n_choices = city_search(fp, search, choices, max_loc);
	if (n_choices == -1 || n_choices >= 100)
	{
		fprintf(stderr, "too many results, be more precise\n");
		getch();
		endwin();
		goto exit_err;
	}
	
	fclose(fp);
	if (n_choices == 0)
	{
		fprintf(stderr, "no search results\n");
		getch();
		endwin();
		goto exit_err;
	}
	
	if (ferror(stdout)) 
	{
		fprintf(stderr, "error writing stdout\n");
		getch();
		endwin();
		goto exit_err;
	}
	
	initscr();
	clear();
	noecho();
	cbreak();
	
	height = n_choices + 5;
	width = (n_choices * 2)+ 5;
	if (width < 50)
		width = 50;
	starty = (LINES - height) / 2;
	startx = (COLS - width) / 2;
	
	menu_win = newwin(height, width, starty, startx);
	keypad(menu_win, TRUE);
	refresh();
	print_menu(menu_win, highlight, choices);
	while(1)
	{
		c = wgetch(menu_win);
		switch(c)
		{
			case 'k':
				if(highlight == 1)
					highlight = n_choices;
				else
					--highlight;
				break;
			case 'j':
				if(highlight == n_choices)
					highlight = 1;
				else
					++highlight;
				break;
			case '\n':
				choice = highlight;
				break;
			default:
				refresh();
				break;
		}
		print_menu(menu_win, highlight, choices);
		if(choice != 0)
			break;
	}
	
	exit_err:
	clear();
	refresh();
	for (int i = 0; i < n_choices; ++i)
	{
		if (choices[i] != NULL)
		{
			free(choices[i]->city);
			free(choices[i]->country);
			free(choices[i]->latitude);
			free(choices[i]->longitude);
			free(choices[i]);
		}
	}
	endwin();
	return 0;
}

