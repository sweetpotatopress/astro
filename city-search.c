#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void city_search(FILE *ifp, char search[])
{
	int c;
	char s[256];
	int line_num = 0;
	
	while (fgets(s, sizeof(s), ifp) != NULL)
	{
		++line_num;
		if (strstr(s, search) != NULL)
			printf("Found %s at %d: %s", search, line_num, s);
	}
}

int main(int argc, char *argv[])
{
	FILE *fp;
	char *prog = argv[0];
	char *search = argv[2];
	
	while (--argc > 1)
		if ((fp = fopen(argv[1], "r")) == NULL)
		{
			fprintf(stderr, "%s: can't open %s\n",
				prog, *argv);
			break;
			exit(1);
		}
		else
		{
			city_search(fp, search);
			fclose(fp);
		}
	if (ferror(stdout)) 
	{
		fprintf(stderr, "%s: error writing stdout\n", prog);
		exit(2);
	}
	exit(0);
}
