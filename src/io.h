/* Copyright (C) 2026 yam lynn
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License
as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTIBILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. if not, see <https://www.gnu.org/licenses/> */

#define FCITY 0
#define FSTATE 1
#define FCOUNTRY 2
#define FYEAR 3
#define FMONTH 4
#define FDAY 5
#define FHOUR 6
#define FMIN 7
#define FTZ 8
#define FLAT 9
#define FLON 10
#define FMAX 11

struct io {
	char *filepath;
	char *filename;
	size_t file_count;
};

void load_chart(struct cdata *cdata, struct io *io,
char *citybuffer, char *statebuffer, char *countrybuffer);
void save_chart(struct cdata *cdata, struct io *io,
char *citybuffer, char *statebuffer, char *countrybuffer);
