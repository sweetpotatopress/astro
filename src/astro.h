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
#pragma once

#include <ncurses.h>
#include <menu.h>
#include <panel.h>
#include <form.h>

#define MAXBUF 1024
#define MAXPATH 2048
#define CHARTMAX 13

#define ERR_EXIT(str) do { \
		endwin(); \
		swe_close(); \
		fprintf(stderr, "\n --?> %s <--\n\n", str); \
		exit(EXIT_FAILURE); \
		} while (0)
		
// planet data
#define LONG 0
#define LAT 1
#define DIST 2
#define LONG_S 3
#define LAT_S 4
#define DIST_S 5
#define RETRO 6
#define STATION 7
#define DEGREE 8
#define MIN 9
#define DEGREE_S 10
#define SIGN 11
#define MIN_S 12
#define NEXT_S 13
#define NEXT_Z 14
#define NEXT_JUL 15
#define PREV_S 16
#define PREV_Z 17
#define PREV_JUL 18
#define RET_INIT 19
#define PL_X 20
#define PL_Y 21
#define MAXPXX 22

#define SPXXMAX 18 // struct member count
struct pxx {
	double dsun[MAXPXX];
	double dmoon[MAXPXX];
	double dmerc[MAXPXX];
	double dven[MAXPXX];
	double dmars[MAXPXX];
	double djup[MAXPXX];
	double dsat[MAXPXX];
	double dura[MAXPXX];
	double dnep[MAXPXX];
	double dplu[MAXPXX];
	double dmnod[MAXPXX];
	double dtnod[MAXPXX];
	double dasc[MAXPXX];
	double dmc[MAXPXX];
	double ddsc[MAXPXX];
	double dic[MAXPXX];
	double dfor[MAXPXX];
	double ddai[MAXPXX];
};

#define E_INIT 0
#define EN_JUL 1
#define EN_FJUL 2
#define EN_OBS 3
#define EN_SIGN 4
#define EP_JUL 5
#define EP_FJUL 6
#define EP_OBS 7
#define EP_SIGN 8
#define EMAX 9

struct cdata {
	char *city;
	char *state;
	char *country;
	char *timezone;
	char *latitude;
	char *longitude;
	char *chart_name;
	double dlat;
	double dlon;
	double utc_hour; //0.0 .. 23.999999;
	double jd_ut;
	int sec;
	int min;
	int hour;
	int mday;
	int mon;
	int year;
	int isdst;
	int wday;
	int utc_year;
	int utc_mon;
	int utc_mday;
	int moonphase;
	bool lt; // LMT flag
	bool dt; // dst reverse
	int rt; // realtime flag
	double cusp[12];
	double sign_cusp[12];
	double t_cusp; // transit cusp
	double le[EMAX]; // lunar eclipse
	double se[EMAX]; // solar eclipse
};

struct ui;

#define CITY 0
#define YEAR 1
#define MONTH 2
#define DAY 3
#define HOUR 4
#define MINUTE 5
#define SECOND 6
#define AMPM 7
#define DRAW 8
#define TIMEZONE 9
#define LATITUDE 10
#define LONGITUDE 11
#define FIELDMAX 12

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
#define FMAX 12

#define SAVE 1
#define LOAD 0

void *ecalloc(size_t n, size_t size);
void *erealloc(void *p, size_t size);
char *xstrdup(const char *s);
char *xstrtok(char *s, const char *delim);
void config_init(struct cdata *cdata, struct ui *ui);
void config_menu(struct cdata *cdata, struct ui *ui);
void in_cdata(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac);
void city_search(struct cdata *cdata,  struct ui *ui, FIELD *cdata_field[], FORM *cdata_form, char *search);
void zodiacal_releasing(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac);
void path_check(char *path, const char *s);
void load_chart(struct cdata *cdata, struct ui *ui);
void save_chart(struct cdata *cdata, struct ui *ui);
void print_fb(struct cdata *cdata, struct ui *ui, char *path, char *head, char ls, bool sl);
void set_chart(struct cdata **cdata, struct pxx **pxx, struct ui *ui, double **planet, int **zodiac, int ch);
void datadir_check(void);
size_t file_count(const char *path, const int r);
size_t name_to_item(char *filepath, ITEM **item, char **name, char **desc);
void back_dir(char *filepath, char *cur_dir);
void diary(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac, int ch);
