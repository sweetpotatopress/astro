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

#define FCITY 0
#define FSTATE 1
#define FCOUNTRY 2
#define FYEAR 3
#define FMONTH 4
#define FDAY 5
#define FHOUR 6
#define FMIN 7
#define FSEC 8
#define FTZ 9
#define FLAT 10
#define FLON 11
#define FDST 12
#define FMAX 13

void xdg_check(struct hd *hd, const char *s);
void load_chart(struct cdata *cdata, struct io *io, struct hd *hd);
void save_chart(struct cdata *cdata, struct io *io, struct hd *hd);
