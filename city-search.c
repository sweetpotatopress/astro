#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

static char *choices[64] = {0};
static int n_choices = 0;
 
int city_search(FILE *ifp, char *search, char **choices)
{
	int i = 0;
	char s[256];
	
	while (fgets(s, sizeof(s), ifp) != NULL)
	{
		if (strstr(s,search) != NULL)
		{
			int len = strlen(s);
			if (s[len - 1] == '\n')
				s[len - 1] = '\0';
				
			choices[i] = malloc(strlen(s) + 1);
			if (choices[i] != NULL)
			{
				strcpy(choices[i], s);
				i++;
			}
		}
	}
	return i;
}

void print_menu(WINDOW *menu_win, int highlight) 
{ 
	int x, y, i;  
	x = 2;
	y = 2;
    box(menu_win, 0, 0); 
    for(i = 0; i < n_choices; ++i)
    {
    	if (highlight == i + 1) 
   		{    
  			wattron(menu_win, A_REVERSE); 
 			mvwprintw(menu_win, y, x, "%s", choices[i]); 
			wattroff(menu_win, A_REVERSE);
		}
		else 
			mvwprintw(menu_win, y, x, "%s", choices[i]); 
		++y; 
	} 
	wrefresh(menu_win); 
} 

int main(int argc, char *argv[])
{
	FILE *fp;
	const char *prog = argv[0];
	const char *path = "world_cities.csv";
	char *search = argv[1];
	WINDOW *menu_win;
	int highlight = 1;
	int choice = 0;
	int c;
	
	
	if (argc > 2)
	{	
		fprintf(stderr, "too many arguments\n");
		exit(-1);
	}
	fp = fopen(path, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "%s: can't open %s\n",
		prog, path);
		exit(1);
	}
	
	n_choices = city_search(fp, search, choices);
	fclose(fp);
	
	if (ferror(stdout)) 
	{
		fprintf(stderr, "%s: error writing stdout\n", prog);
		exit(2);
	}
	
	initscr();
	clear();
	noecho();
	cbreak();
	
	menu_win = newwin(0, 0, 0, 0);
	keypad(menu_win, TRUE);
	refresh();
	print_menu(menu_win, highlight);
	while(1)
	{
		c = wgetch(menu_win);
		switch(c)
		{
			case KEY_UP:
				if(highlight == 1)
					highlight = n_choices;
				else
					--highlight;
				break;
			case KEY_DOWN:
				if(highlight == n_choices)
					highlight = 1;
				else
					++highlight;
				break;
			case 10:
				choice = highlight;
				break;
			default:
				refresh();
				break;
		}
		print_menu(menu_win, highlight);
		if(choice != 0)
			break;
	}
	mvprintw(23, 0, "your choice: %s", choices[choice - 1]);
	clrtoeol();
	refresh();
	getch();
	for (int i = 0; i < n_choices; ++i)
	{
		if (choices[i] != NULL)
			free(choices[i]);
	}
	endwin();
	
	exit(0);
}

