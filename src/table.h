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

#define ELEMENT 0
#define RULER 1
#define EXALT 2
#define TRIPLD 3
#define TRIPLN 4
#define TRIPLC 5
#define BOUND0 6
#define BOUND1 7
#define BOUND2 8
#define BOUND3 9
#define BOUND4 10
#define DECAN0 11
#define DECAN1 12
#define DECAN2 13
#define DETRI 14
#define FALL 15
#define EMPTY 16
#define MAXZXX 17

#define PLMAX 16

struct zxx {
	int iari[MAXZXX];
	int itau[MAXZXX];
	int igem[MAXZXX];
	int ican[MAXZXX];
	int ileo[MAXZXX];
	int ivir[MAXZXX];
	int ilib[MAXZXX];
	int isco[MAXZXX];
	int isag[MAXZXX];
	int icap[MAXZXX];
	int iaqu[MAXZXX];
	int ipis[MAXZXX];
};
