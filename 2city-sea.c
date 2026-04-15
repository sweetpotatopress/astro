#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

static int startx = 0;
static int starty = 0;
char *choices[4] = {0};
static int n_choices = sizeof(choices) / sizeof(char *);
 
void city_search(FILE *ifp, char *search, char **choices)
{
	int c, i = 0;
	char s[256];
	int line_num = 0;
	
	while (fgets(s, sizeof(s), ifp) != NULL)
	{
		++line_num;
		if (strstr(s,search) != NULL)
		{
			choices[i++] = s;
		}
	}
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
	char *prog = argv[0];
	char *path = "world_cities.csv";
	char *search = argv[1];
	WINDOW *menu_win;
	int highlight = 1;
	int choice = 0;
	int c;
	

	while (--argc > 0)
		if ((fp = fopen(path, "r")) == NULL)
		{
			fprintf(stderr, "%s: can't open %s\n",
				prog, *argv);
			break;
			exit(1);
		}
		else
		{
			city_search(fp, search, choices);
		}
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
				mvprintw(24, 0, "char presses is = %3d hopefully it can be printed as %c", c, c);
				refresh();
				break;
		}
		print_menu(menu_win, highlight);
		if(choice != 0)
			break;
	}
	mvprintw(23, 0, "you choose choice %d wit choice %s\n", choice, choices[choice - 1]);
	clrtoeol();
	refresh();
	getch();
	endwin();
	
	exit(0);
}

