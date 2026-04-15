#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

typedef struct {
	char *city;
	char *country;
	char *timezone;
	char *latitude;
	char *longitude;
} Location;

static int n_choices = 0;
char *fields[19];
 
int city_search(FILE *ifp, char *search, Location **choices)
{
	int i = 0;
	char line[1024];
	
	while (fgets(line, sizeof(line), ifp) != NULL)
	{
		int len = strlen(line);
		if (line[len - 1] == '\n')
			line[len - 1] = '\0';
		
		char *copy = malloc(strlen(line) + 1);
		strcpy(copy, line);
		
		char *token = strtok(copy, "\t");
		int field_count = 0;
				
		while (token != NULL && field_count < 19)
		{
			fields[field_count++] = token;
			token = strtok(NULL, "\t");
		}
		
		if (field_count > 1 && strstr(fields[1], search) != NULL)
		{
			Location *location = malloc(sizeof(Location));
			location->city = malloc(strlen(fields[1]) + 1);
			strcpy(location->city, fields[1]);
			location->latitude = malloc(strlen(fields[6]) + 1);
			strcpy(location->latitude, fields[6]);
			choices[i++] = location;
		}
		free(copy);
	}
	return i;
}

void print_menu(WINDOW *menu_win, int highlight, Location **choices)
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
 			mvwprintw(menu_win, y, x, "%s", choices[i]->city);
			wattroff(menu_win, A_REVERSE);
		}
		else 
			mvwprintw(menu_win, y, x, "%s", choices[i]->city); 
		++y; 
	} 
	wrefresh(menu_win); 
} 

int main(int argc, char *argv[])
{
	FILE *fp;
	const char *prog = argv[0];
	const char *path = "cities15000.txt";
	char *search = argv[1];
	WINDOW *menu_win;
	int highlight = 1;
	int choice = 0;
	int c;
	int max_loc = 100;
	Location **choices = malloc(sizeof(Location *) * max_loc);
	
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
	if (n_choices == 0)
	{
		fprintf(stderr, "no search results\n");
		free(choices);
		endwin();
		exit(1);
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

