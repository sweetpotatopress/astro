/*This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License
as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTIBILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. if not, see <https://www.gnu.org/licenses/> */

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include "swephexp.h"
#include "astro.h"
#include "io.h"

#define TZ_DEF 0
#define LAT_DEF 1
#define LON_DEF 2
#define MAX_DEF 3

void config_parse(struct cdata *cdata, char xdg_path[])
{
	xdg_check(xdg_path, "config");
	
	FILE *fp = fopen(xdg_path, "r");
	if (fp == NULL)
		return;
		
	char buffer[MAXBUF] = {0};
	char field[MAX_DEF][MAXBUF] = {0};
	size_t i = 0, c = 0;
		
	while (fgets(buffer, sizeof(buffer), fp) != NULL)
	{
		for (i = 0; i < strlen(buffer) + 1; ++i)
			if (buffer[i] == '=')
			{
				i += 2;
				for (int j = 0; i < strlen(buffer) + 1; j++, i++)
				{
					if (buffer[i] == '\n')
						buffer[i] = '\0';
					field[c][j] = buffer[i];
				}
			}
		++c;
	}
	fclose(fp);
	
	if (setenv("TZ", field[TZ_DEF], 1) != 0)
		ERR_EXIT("ERR: setenv conf.c");
	tzset();
	memcpy(cdata->timezone,
	field[TZ_DEF], strlen(field[TZ_DEF]) + 1);
	
	char *endptr = NULL;
	double dret;
	errno = 0;
	
	dret = strtod(field[LAT_DEF], &endptr);
	if (errno != ERANGE)
		cdata->dlat = dret;
	dret = strtod(field[LON_DEF], &endptr);
	if (errno != ERANGE)
		cdata->dlon = dret;
}
