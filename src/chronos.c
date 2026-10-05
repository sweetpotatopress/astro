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

#include <math.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <ncurses.h>
#include "swephexp.h"
#include "astro.h"
#include "ui.h"
#include "init.h"
#include "chronos.h"

#ifdef __GLIBC__
#define TM_GMTOFF(x) ((x).__tm_gmtoff)
#else
#define TM_GMTOFF(x) ((x).tm_gmtoff)
#endif

void set_localtime(struct cdata *cdata)
{	
	time_t now = time(NULL);
	struct tm gt = {0};
		
	localtime_r(&now, &gt);
	
	cdata->year = gt.tm_year+1900;
	cdata->mon = gt.tm_mon + 1;
	cdata->mday = gt.tm_mday;
	cdata->hour = gt.tm_hour;
	cdata->min = gt.tm_min;
	cdata->sec = gt.tm_sec;
	cdata->wday = gt.tm_wday;
	cdata->isdst = gt.tm_isdst;
}

void weekday_check(struct cdata *cdata)
{
	struct tm gt = {0};
	
	gt.tm_year = cdata->year - 1900;
	gt.tm_mon = cdata->mon - 1;
	gt.tm_mday = cdata->mday;
	gt.tm_hour = cdata->hour - 1;
	gt.tm_min = cdata->min;
	gt.tm_sec = cdata->sec;
	
	mktime(&gt);
	cdata->wday = gt.tm_wday;
}

int daycount(int month, int year)
{
	const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	if (month == 2)
		if (((year + 1900) % 4 == 0 && (year + 1900) % 100 != 0) || 
		((year + 1900) % 400 == 0))
			return 29;
	return days[month];
}

int sect(struct pxx *pxx)
{
	int sect = 0;
	double dist = pxx->dsun[LONG] - pxx->dasc[LONG];
	
	while (dist < 0.0)
		dist += 360.0;
	while (dist >= 360.0)
		dist -= 360.0;
	
	return sect = (dist > 180.0) ? DAY_SECT : NIGHT_SECT;
}

void lots(struct pxx *pxx)
{
	int chart_sect = sect(pxx);
	double offset = (360 - pxx->dsun[LONG]);
	double diff = (offset + pxx->dmoon[LONG]);
	while (diff > 360.0)
		diff -= 360.0;
	
	if (chart_sect == DAY_SECT)
	{
		pxx->dfor[LONG] = pxx->dasc[LONG] + diff;
		pxx->dspir[LONG] = pxx->dasc[LONG] - diff;
	}
	else // night
	{
		pxx->dfor[LONG] = pxx->dasc[LONG] - diff;
		pxx->dspir[LONG] = pxx->dasc[LONG] + diff;
	}
	
	while (pxx->dfor[LONG] < 0.0)
		pxx->dfor[LONG] += 360.0;
	while (pxx->dfor[LONG] > 360.0)
		pxx->dfor[LONG] -= 360.0;
		
	while (pxx->dspir[LONG] < 0.0)
		pxx->dspir[LONG] += 360.0;
	while (pxx->dspir[LONG] > 360.0)
		pxx->dspir[LONG] -= 360.0;
}

struct lmt {
    const char *timezone;
    int year;
    int mon;
    int mday;
    int hour;
    int min;
    int sec;
};

static const struct lmt dates[] = {
    { "Africa/Abidjan",  1912,  1,  1,  0, 16,  8 },
    { "Africa/Accra",  1912,  1,  1,  0, 16,  8 },
    { "Africa/Addis_Ababa",  1908,  5,  1,  0,  2, 44 },
    { "Africa/Algiers",  1891,  3, 15, 23, 57,  9 },
    { "Africa/Asmara",  1908,  5,  1,  0,  2, 44 },
    { "Africa/Asmera",  1908,  5,  1,  0,  2, 44 },
    { "Africa/Bamako",  1912,  1,  1,  0, 16,  8 },
    { "Africa/Bangui",  1905,  6, 30, 23, 46, 25 },
    { "Africa/Banjul",  1912,  1,  1,  0, 16,  8 },
    { "Africa/Bissau",  1912,  1,  1,  0,  0,  0 },
    { "Africa/Blantyre",  1908, 12, 31, 23, 49, 42 },
    { "Africa/Brazzaville",  1905,  6, 30, 23, 46, 25 },
    { "Africa/Bujumbura",  1908, 12, 31, 23, 49, 42 },
    { "Africa/Cairo",  1900,  9, 30, 23, 54, 51 },
    { "Africa/Casablanca",  1913, 10, 26,  0, 30, 20 },
    { "Africa/Ceuta",  1901,  1,  1,  0,  0,  0 },
    { "Africa/Conakry",  1912,  1,  1,  0, 16,  8 },
    { "Africa/Dakar",  1912,  1,  1,  0, 16,  8 },
    { "Africa/Dar_es_Salaam",  1908,  5,  1,  0,  2, 44 },
    { "Africa/Djibouti",  1908,  5,  1,  0,  2, 44 },
    { "Africa/Douala",  1905,  6, 30, 23, 46, 25 },
    { "Africa/El_Aaiun",  1933, 12, 31, 23, 52, 48 },
    { "Africa/Freetown",  1912,  1,  1,  0, 16,  8 },
    { "Africa/Gaborone",  1908, 12, 31, 23, 49, 42 },
    { "Africa/Harare",  1908, 12, 31, 23, 49, 42 },
    { "Africa/Johannesburg",  1892,  2,  7, 23, 38,  0 },
    { "Africa/Juba",  1930, 12, 31, 23, 53, 32 },
    { "Africa/Kampala",  1908,  5,  1,  0,  2, 44 },
    { "Africa/Khartoum",  1930, 12, 31, 23, 49, 52 },
    { "Africa/Kigali",  1908, 12, 31, 23, 49, 42 },
    { "Africa/Kinshasa",  1905,  6, 30, 23, 46, 25 },
    { "Africa/Lagos",  1905,  6, 30, 23, 46, 25 },
    { "Africa/Libreville",  1905,  6, 30, 23, 46, 25 },
    { "Africa/Lome",  1912,  1,  1,  0, 16,  8 },
    { "Africa/Luanda",  1905,  6, 30, 23, 46, 25 },
    { "Africa/Lubumbashi",  1908, 12, 31, 23, 49, 42 },
    { "Africa/Lusaka",  1908, 12, 31, 23, 49, 42 },
    { "Africa/Malabo",  1905,  6, 30, 23, 46, 25 },
    { "Africa/Maputo",  1908, 12, 31, 23, 49, 42 },
    { "Africa/Maseru",  1892,  2,  7, 23, 38,  0 },
    { "Africa/Mbabane",  1892,  2,  7, 23, 38,  0 },
    { "Africa/Mogadishu",  1908,  5,  1,  0,  2, 44 },
    { "Africa/Monrovia",  1882,  1,  1,  0,  0,  0 },
    { "Africa/Nairobi",  1908,  5,  1,  0,  2, 44 },
    { "Africa/Ndjamena",  1911, 12, 31, 23, 59, 48 },
    { "Africa/Niamey",  1905,  6, 30, 23, 46, 25 },
    { "Africa/Nouakchott",  1912,  1,  1,  0, 16,  8 },
    { "Africa/Ouagadougou",  1912,  1,  1,  0, 16,  8 },
    { "Africa/Porto-Novo",  1905,  6, 30, 23, 46, 25 },
    { "Africa/Sao_Tome",  1912,  1,  1,  0,  0,  0 },
    { "Africa/Timbuktu",  1912,  1,  1,  0, 16,  8 },
    { "Africa/Tripoli",  1920,  1,  1,  0,  7, 16 },
    { "Africa/Tunis",  1881,  5, 11, 23, 28, 37 },
    { "Africa/Windhoek",  1892,  2,  8,  0, 21, 36 },
    { "America/Adak",  1900,  8, 20, 12, 46, 38 },
    { "America/Anchorage",  1900,  8, 20, 11, 59, 36 },
    { "America/Anguilla",  1899,  3, 28, 12, 24, 25 },
    { "America/Antigua",  1899,  3, 28, 12, 24, 25 },
    { "America/Araguaina",  1914,  1,  1,  0, 12, 48 },
    { "America/Argentina/Buenos_Aires",  1894, 10, 30, 23, 37,  0 },
    { "America/Argentina/Catamarca",  1894, 10, 31,  0,  6, 20 },
    { "America/Argentina/ComodRivadavia",  1894, 10, 31,  0,  6, 20 },
    { "America/Argentina/Cordoba",  1894, 10, 31,  0,  0,  0 },
    { "America/Argentina/Jujuy",  1894, 10, 31,  0,  4, 24 },
    { "America/Argentina/La_Rioja",  1894, 10, 31,  0, 10, 36 },
    { "America/Argentina/Mendoza",  1894, 10, 31,  0, 18, 28 },
    { "America/Argentina/Rio_Gallegos",  1894, 10, 31,  0, 20,  4 },
    { "America/Argentina/Salta",  1894, 10, 31,  0,  4, 52 },
    { "America/Argentina/San_Juan",  1894, 10, 31,  0, 17, 16 },
    { "America/Argentina/San_Luis",  1894, 10, 31,  0,  8, 36 },
    { "America/Argentina/Tucuman",  1894, 10, 31,  0,  4,  4 },
    { "America/Argentina/Ushuaia",  1894, 10, 31,  0, 16, 24 },
    { "America/Aruba",  1899,  3, 28, 12, 24, 25 },
    { "America/Asuncion",  1890,  1,  1,  0,  0,  0 },
    { "America/Atikokan",  1889, 12, 31, 23, 58, 32 },
    { "America/Atka",  1900,  8, 20, 12, 46, 38 },
    { "America/Bahia",  1913, 12, 31, 23, 34,  4 },
    { "America/Bahia_Banderas",  1922,  1,  1,  0,  0,  0 },
    { "America/Barbados",  1911,  8, 27, 23, 58, 29 },
    { "America/Belem",  1914,  1,  1,  0, 13, 56 },
    { "America/Belize",  1912,  3, 31, 23, 52, 48 },
    { "America/Blanc-Sablon",  1899,  3, 28, 12, 24, 25 },
    { "America/Boa_Vista",  1914,  1,  1,  0,  2, 40 },
    { "America/Bogota",  1884,  3, 13,  0,  0,  0 },
    { "America/Boise",  1883, 11, 18, 12,  0,  0 },
    { "America/Buenos_Aires",  1894, 10, 30, 23, 37,  0 },
    { "America/Campo_Grande",  1913, 12, 31, 23, 38, 28 },
    { "America/Cancun",  1922,  1,  1,  0,  0,  0 },
    { "America/Caracas",  1890,  1,  1,  0,  0,  4 },
    { "America/Catamarca",  1894, 10, 31,  0,  6, 20 },
    { "America/Cayenne",  1911,  6, 30, 23, 29, 20 },
    { "America/Cayman",  1889, 12, 31, 23, 58, 32 },
    { "America/Chicago",  1883, 11, 18, 12,  0,  0 },
    { "America/Chihuahua",  1922,  1,  1,  0,  0,  0 },
    { "America/Ciudad_Juarez",  1922,  1,  1,  0,  0,  0 },
    { "America/Coral_Harbour",  1889, 12, 31, 23, 58, 32 },
    { "America/Cordoba",  1894, 10, 31,  0,  0,  0 },
    { "America/Costa_Rica",  1890,  1,  1,  0,  0,  0 },
    { "America/Coyhaique",  1890,  1,  1,  0,  5, 31 },
    { "America/Creston",  1883, 11, 18, 12,  0,  0 },
    { "America/Cuiaba",  1913, 12, 31, 23, 44, 20 },
    { "America/Curacao",  1899,  3, 28, 12, 24, 25 },
    { "America/Danmarkshavn",  1916,  7, 27, 22, 14, 40 },
    { "America/Dawson",  1900,  8, 20,  0, 17, 40 },
    { "America/Dawson_Creek",  1884,  1,  1,  0,  0, 56 },
    { "America/Denver",  1883, 11, 18, 12,  0,  0 },
    { "America/Detroit",  1904, 12, 31, 23, 32, 11 },
    { "America/Dominica",  1899,  3, 28, 12, 24, 25 },
    { "America/Edmonton",  1906,  9,  1,  0, 33, 52 },
    { "America/Eirunepe",  1913, 12, 31, 23, 39, 28 },
    { "America/El_Salvador",  1920, 12, 31, 23, 56, 48 },
    { "America/Ensenada",  1922,  1,  1,  0,  0,  0 },
    { "America/Fort_Nelson",  1884,  1,  1,  0, 10, 47 },
    { "America/Fort_Wayne",  1883, 11, 18, 12,  0,  0 },
    { "America/Fortaleza",  1913, 12, 31, 23, 34,  0 },
    { "America/Glace_Bay",  1902,  6, 14, 23, 59, 48 },
    { "America/Godthab",  1916,  7, 28,  0, 26, 56 },
    { "America/Goose_Bay",  1884,  1,  1,  0, 30, 48 },
    { "America/Grand_Turk",  1889, 12, 31, 23, 37, 22 },
    { "America/Grenada",  1899,  3, 28, 12, 24, 25 },
    { "America/Guadeloupe",  1899,  3, 28, 12, 24, 25 },
    { "America/Guatemala",  1918, 10,  5,  0,  2,  4 },
    { "America/Guayaquil",  1890,  1,  1,  0,  5, 20 },
    { "America/Guyana",  1911,  7, 31, 23, 52, 39 },
    { "America/Halifax",  1902,  6, 15,  0, 14, 24 },
    { "America/Havana",  1889, 12, 31, 23, 59, 52 },
    { "America/Hermosillo",  1922,  1,  1,  0,  0,  0 },
    { "America/Indiana/Indianapolis",  1883, 11, 18, 12,  0,  0 },
    { "America/Indiana/Knox",  1883, 11, 18, 12,  0,  0 },
    { "America/Indiana/Marengo",  1883, 11, 18, 12,  0,  0 },
    { "America/Indiana/Petersburg",  1883, 11, 18, 12,  0,  0 },
    { "America/Indiana/Tell_City",  1883, 11, 18, 12,  0,  0 },
    { "America/Indiana/Vevay",  1883, 11, 18, 12,  0,  0 },
    { "America/Indiana/Vincennes",  1883, 11, 18, 12,  0,  0 },
    { "America/Indiana/Winamac",  1883, 11, 18, 12,  0,  0 },
    { "America/Indianapolis",  1883, 11, 18, 12,  0,  0 },
    { "America/Jamaica",  1890,  1,  1,  0,  0,  0 },
    { "America/Jujuy",  1894, 10, 31,  0,  4, 24 },
    { "America/Juneau",  1900,  8, 20, 12, 57, 41 },
    { "America/Kentucky/Louisville",  1883, 11, 18, 12,  0,  0 },
    { "America/Kentucky/Monticello",  1883, 11, 18, 12,  0,  0 },
    { "America/Knox_IN",  1883, 11, 18, 12,  0,  0 },
    { "America/Kralendijk",  1899,  3, 28, 12, 24, 25 },
    { "America/La_Paz",  1890,  1,  1,  0,  0,  0 },
    { "America/Lima",  1908,  7, 28,  0,  8, 36 },
    { "America/Los_Angeles",  1883, 11, 18, 12,  0,  0 },
    { "America/Louisville",  1883, 11, 18, 12,  0,  0 },
    { "America/Lower_Princes",  1899,  3, 28, 12, 24, 25 },
    { "America/Maceio",  1913, 12, 31, 23, 22, 52 },
    { "America/Managua",  1889, 12, 31, 23, 59, 56 },
    { "America/Manaus",  1914,  1,  1,  0,  0,  4 },
    { "America/Marigot",  1899,  3, 28, 12, 24, 25 },
    { "America/Martinique",  1890,  1,  1,  0,  0,  0 },
    { "America/Matamoros",  1922,  1,  1,  0,  0,  0 },
    { "America/Mazatlan",  1922,  1,  1,  0,  0,  0 },
    { "America/Mendoza",  1894, 10, 31,  0, 18, 28 },
    { "America/Menominee",  1885,  9, 18, 11, 50, 27 },
    { "America/Merida",  1922,  1,  1,  0,  0,  0 },
    { "America/Metlakatla",  1900,  8, 20, 12, 46, 18 },
    { "America/Mexico_City",  1922,  1,  1,  0,  0,  0 },
    { "America/Miquelon",  1911,  6, 14, 23, 44, 40 },
    { "America/Moncton",  1883, 12,  8, 23, 19,  8 },
    { "America/Monterrey",  1921, 12, 31, 23,  0,  0 },
    { "America/Montevideo",  1908,  6, 10,  0,  0,  0 },
    { "America/Montreal",  1895,  1,  1,  0, 17, 32 },
    { "America/Montserrat",  1899,  3, 28, 12, 24, 25 },
    { "America/Nassau",  1895,  1,  1,  0, 17, 32 },
    { "America/New_York",  1883, 11, 18, 12,  0,  0 },
    { "America/Nipigon",  1895,  1,  1,  0, 17, 32 },
    { "America/Nome",  1900,  8, 20, 12,  1, 38 },
    { "America/Noronha",  1914,  1,  1,  0,  9, 40 },
    { "America/North_Dakota/Beulah",  1883, 11, 18, 12,  0,  0 },
    { "America/North_Dakota/Center",  1883, 11, 18, 12,  0,  0 },
    { "America/North_Dakota/New_Salem",  1883, 11, 18, 12,  0,  0 },
    { "America/Nuuk",  1916,  7, 28,  0, 26, 56 },
    { "America/Ojinaga",  1922,  1,  1,  0,  0,  0 },
    { "America/Panama",  1889, 12, 31, 23, 58, 32 },
    { "America/Paramaribo",  1910, 12, 31, 23, 59, 48 },
    { "America/Phoenix",  1883, 11, 18, 12,  0,  0 },
    { "America/Port-au-Prince",  1890,  1,  1,  0,  0, 20 },
    { "America/Port_of_Spain",  1899,  3, 28, 12, 24, 25 },
    { "America/Porto_Acre",  1913, 12, 31, 23, 31, 12 },
    { "America/Porto_Velho",  1914,  1,  1,  0, 15, 36 },
    { "America/Puerto_Rico",  1899,  3, 28, 12, 24, 25 },
    { "America/Punta_Arenas",  1890,  1,  1,  0,  0, 55 },
    { "America/Rainy_River",  1887,  7, 16,  0, 28, 36 },
    { "America/Recife",  1913, 12, 31, 23, 19, 36 },
    { "America/Regina",  1905,  8, 31, 23, 58, 36 },
    { "America/Rio_Branco",  1913, 12, 31, 23, 31, 12 },
    { "America/Rosario",  1894, 10, 31,  0,  0,  0 },
    { "America/Santa_Isabel",  1922,  1,  1,  0,  0,  0 },
    { "America/Santarem",  1913, 12, 31, 23, 38, 48 },
    { "America/Santiago",  1890,  1,  1,  0,  0,  0 },
    { "America/Santo_Domingo",  1889, 12, 31, 23, 59, 36 },
    { "America/Sao_Paulo",  1914,  1,  1,  0,  6, 28 },
    { "America/Scoresbysund",  1916,  7, 27, 23, 27, 52 },
    { "America/Shiprock",  1883, 11, 18, 12,  0,  0 },
    { "America/Sitka",  1900,  8, 20, 13,  1, 13 },
    { "America/St_Barthelemy",  1899,  3, 28, 12, 24, 25 },
    { "America/St_Johns",  1884,  1,  1,  0,  0,  0 },
    { "America/St_Kitts",  1899,  3, 28, 12, 24, 25 },
    { "America/St_Lucia",  1899,  3, 28, 12, 24, 25 },
    { "America/St_Thomas",  1899,  3, 28, 12, 24, 25 },
    { "America/St_Vincent",  1899,  3, 28, 12, 24, 25 },
    { "America/Swift_Current",  1905,  9,  1,  0, 11, 20 },
    { "America/Tegucigalpa",  1921,  3, 31, 23, 48, 52 },
    { "America/Thule",  1916,  7, 28,  0, 35,  8 },
    { "America/Thunder_Bay",  1895,  1,  1,  0, 17, 32 },
    { "America/Tijuana",  1922,  1,  1,  0,  0,  0 },
    { "America/Toronto",  1895,  1,  1,  0, 17, 32 },
    { "America/Tortola",  1899,  3, 28, 12, 24, 25 },
    { "America/Vancouver",  1884,  1,  1,  0, 12, 28 },
    { "America/Virgin",  1899,  3, 28, 12, 24, 25 },
    { "America/Whitehorse",  1900,  8, 20,  0,  0, 12 },
    { "America/Winnipeg",  1887,  7, 16,  0, 28, 36 },
    { "America/Yakutat",  1900,  8, 20, 12, 18, 55 },
    { "America/Yellowknife",  1906,  9,  1,  0, 33, 52 },
    { "Antarctica/DumontDUrville",  1879, 12, 31, 23, 59, 52 },
    { "Antarctica/McMurdo",  1868, 11,  1, 23, 50, 56 },
    { "Antarctica/South_Pole",  1868, 11,  1, 23, 50, 56 },
    { "Antarctica/Syowa",  1947,  3, 13, 23, 53,  8 },
    { "Arctic/Longyearbyen",  1893,  4,  1,  0,  6, 32 },
    { "Asia/Aden",  1947,  3, 13, 23, 53,  8 },
    { "Asia/Almaty",  1924,  5,  1, 23, 52, 12 },
    { "Asia/Amman",  1930, 12, 31, 23, 36, 16 },
    { "Asia/Anadyr",  1924,  5,  2,  0, 10,  4 },
    { "Asia/Aqtau",  1924,  5,  2,  0, 38, 56 },
    { "Asia/Aqtobe",  1924,  5,  2,  0, 11, 20 },
    { "Asia/Ashgabat",  1924,  5,  2,  0,  6, 28 },
    { "Asia/Ashkhabad",  1924,  5,  2,  0,  6, 28 },
    { "Asia/Atyrau",  1924,  5,  1, 23, 32, 16 },
    { "Asia/Baghdad",  1889, 12, 31, 23, 59, 56 },
    { "Asia/Bahrain",  1920,  1,  1,  0, 33, 52 },
    { "Asia/Baku",  1924,  5,  1, 23, 40, 36 },
    { "Asia/Bangkok",  1880,  1,  1,  0,  0,  0 },
    { "Asia/Barnaul",  1919, 12, 10,  0, 25,  0 },
    { "Asia/Beirut",  1879, 12, 31, 23, 38,  0 },
    { "Asia/Bishkek",  1924,  5,  2,  0,  1, 36 },
    { "Asia/Brunei",  1926,  3,  1,  0,  8, 40 },
    { "Asia/Calcutta",  1854,  6, 27, 23, 59, 52 },
    { "Asia/Chita",  1919, 12, 15,  0, 26,  8 },
    { "Asia/Choibalsan",  1905,  7, 31, 23, 52, 28 },
    { "Asia/Chongqing",  1900, 12, 31, 23, 54, 17 },
    { "Asia/Chungking",  1900, 12, 31, 23, 54, 17 },
    { "Asia/Colombo",  1880,  1,  1,  0,  0,  8 },
    { "Asia/Dacca",  1889, 12, 31, 23, 51, 40 },
    { "Asia/Damascus",  1919, 12, 31, 23, 34, 48 },
    { "Asia/Dhaka",  1889, 12, 31, 23, 51, 40 },
    { "Asia/Dili",  1912,  1,  1,  0,  0,  0 },
    { "Asia/Dubai",  1920,  1,  1,  0, 18, 48 },
    { "Asia/Dushanbe",  1924,  5,  2,  0, 24, 48 },
    { "Asia/Famagusta",  1921, 11, 13, 23, 44, 12 },
    { "Asia/Gaza",  1900,  9, 30, 23, 42,  8 },
    { "Asia/Harbin",  1900, 12, 31, 23, 54, 17 },
    { "Asia/Hebron",  1900,  9, 30, 23, 39, 37 },
    { "Asia/Ho_Chi_Minh",  1906,  7,  1,  0,  0,  0 },
    { "Asia/Hong_Kong",  1904, 10, 30,  1,  0,  0 },
    { "Asia/Hovd",  1905,  7, 31, 23, 53, 24 },
    { "Asia/Irkutsk",  1880,  1,  1,  0,  0,  0 },
    { "Asia/Istanbul",  1880,  1,  1,  0,  1,  4 },
    { "Asia/Jakarta",  1867,  8, 10,  0,  0,  0 },
    { "Asia/Jayapura",  1932, 10, 31, 23, 37, 12 },
    { "Asia/Jerusalem",  1879, 12, 31, 23, 59, 46 },
    { "Asia/Kabul",  1889, 12, 31, 23, 23, 12 },
    { "Asia/Kamchatka",  1922, 11, 10,  0, 25, 24 },
    { "Asia/Karachi",  1907,  1,  1,  1,  1, 48 },
    { "Asia/Kashgar",  1928,  1,  1,  0,  9, 40 },
    { "Asia/Kathmandu",  1919, 12, 31, 23, 48, 44 },
    { "Asia/Katmandu",  1919, 12, 31, 23, 48, 44 },
    { "Asia/Khandyga",  1919, 12, 14, 22, 57, 47 },
    { "Asia/Kolkata",  1854,  6, 27, 23, 59, 52 },
    { "Asia/Krasnoyarsk",  1920,  1,  5, 23, 48, 34 },
    { "Asia/Kuala_Lumpur",  1901,  1,  1,  0,  0,  0 },
    { "Asia/Kuching",  1926,  3,  1,  0,  8, 40 },
    { "Asia/Kuwait",  1947,  3, 13, 23, 53,  8 },
    { "Asia/Macao",  1904, 10, 30,  0, 25, 50 },
    { "Asia/Macau",  1904, 10, 30,  0, 25, 50 },
    { "Asia/Magadan",  1924,  5,  1, 23, 56, 48 },
    { "Asia/Makassar",  1920,  1,  1,  0,  0,  0 },
    { "Asia/Manila",  1899,  9,  6, 12,  0,  0 },
    { "Asia/Muscat",  1920,  1,  1,  0, 18, 48 },
    { "Asia/Nicosia",  1921, 11, 13, 23, 46, 32 },
    { "Asia/Novokuznetsk",  1924,  5,  1,  0, 11, 12 },
    { "Asia/Novosibirsk",  1919, 12, 14,  6, 28, 20 },
    { "Asia/Omsk",  1919, 11, 14,  0,  6, 30 },
    { "Asia/Oral",  1924,  5,  1, 23, 34, 36 },
    { "Asia/Phnom_Penh",  1880,  1,  1,  0,  0,  0 },
    { "Asia/Pontianak",  1908,  5,  1,  0,  0,  0 },
    { "Asia/Pyongyang",  1908,  4,  1,  0,  7,  0 },
    { "Asia/Qatar",  1920,  1,  1,  0, 33, 52 },
    { "Asia/Qostanay",  1924,  5,  1, 23, 45, 32 },
    { "Asia/Qyzylorda",  1924,  5,  1, 23, 38,  8 },
    { "Asia/Rangoon",  1880,  1,  1,  0,  0,  0 },
    { "Asia/Riyadh",  1947,  3, 13, 23, 53,  8 },
    { "Asia/Saigon",  1906,  7,  1,  0,  0,  0 },
    { "Asia/Sakhalin",  1905,  8, 22, 23, 29, 12 },
    { "Asia/Samarkand",  1924,  5,  1, 23, 32,  7 },
    { "Asia/Seoul",  1908,  4,  1,  0,  2,  8 },
    { "Asia/Shanghai",  1900, 12, 31, 23, 54, 17 },
    { "Asia/Singapore",  1901,  1,  1,  0,  0,  0 },
    { "Asia/Srednekolymsk",  1924,  5,  1, 23, 45,  8 },
    { "Asia/Taipei",  1895, 12, 31, 23, 54,  0 },
    { "Asia/Tashkent",  1924,  5,  2,  0, 22, 49 },
    { "Asia/Tbilisi",  1880,  1,  1,  0,  0,  0 },
    { "Asia/Tehran",  1916,  1,  1,  0,  0,  0 },
    { "Asia/Tel_Aviv",  1879, 12, 31, 23, 59, 46 },
    { "Asia/Thimbu",  1947,  8, 14, 23, 31, 24 },
    { "Asia/Thimphu",  1947,  8, 14, 23, 31, 24 },
    { "Asia/Tokyo",  1888,  1,  1,  0,  0,  0 },
    { "Asia/Tomsk",  1919, 12, 22,  0, 20,  9 },
    { "Asia/Ujung_Pandang",  1920,  1,  1,  0,  0,  0 },
    { "Asia/Ulaanbaatar",  1905,  7, 31, 23, 52, 28 },
    { "Asia/Ulan_Bator",  1905,  7, 31, 23, 52, 28 },
    { "Asia/Urumqi",  1928,  1,  1,  0,  9, 40 },
    { "Asia/Ust-Nera",  1919, 12, 14, 22, 27,  6 },
    { "Asia/Vientiane",  1880,  1,  1,  0,  0,  0 },
    { "Asia/Vladivostok",  1922, 11, 15,  0, 12, 29 },
    { "Asia/Yakutsk",  1919, 12, 14, 23, 21,  2 },
    { "Asia/Yangon",  1880,  1,  1,  0,  0,  0 },
    { "Asia/Yekaterinburg",  1916,  7,  2, 23, 42, 32 },
    { "Asia/Yerevan",  1924,  5,  2,  0,  2,  0 },
    { "Atlantic/Azores",  1883, 12, 31, 23, 48,  8 },
    { "Atlantic/Bermuda",  1890,  1,  1,  0,  0,  0 },
    { "Atlantic/Canary",  1922,  3,  1,  0,  1, 36 },
    { "Atlantic/Cape_Verde",  1912,  1,  1,  0,  0,  0 },
    { "Atlantic/Faeroe",  1908,  1, 11,  0, 27,  4 },
    { "Atlantic/Faroe",  1908,  1, 11,  0, 27,  4 },
    { "Atlantic/Jan_Mayen",  1893,  4,  1,  0,  6, 32 },
    { "Atlantic/Madeira",  1884,  1,  1,  0,  0,  0 },
    { "Atlantic/Reykjavik",  1912,  1,  1,  0, 16,  8 },
    { "Atlantic/South_Georgia",  1890,  1,  1,  0, 26,  8 },
    { "Atlantic/St_Helena",  1912,  1,  1,  0, 16,  8 },
    { "Atlantic/Stanley",  1890,  1,  1,  0,  0,  0 },
    { "Australia/ACT",  1895,  1, 31, 23, 55,  8 },
    { "Australia/Adelaide",  1895,  1, 31, 23, 45, 40 },
    { "Australia/Brisbane",  1894, 12, 31, 23, 47, 52 },
    { "Australia/Broken_Hill",  1895,  2,  1,  0, 34, 12 },
    { "Australia/Canberra",  1895,  1, 31, 23, 55,  8 },
    { "Australia/Currie",  1895,  9,  1,  0, 10, 44 },
    { "Australia/Darwin",  1895,  2,  1,  0, 16, 40 },
    { "Australia/Eucla",  1895, 12,  1,  0,  9, 32 },
    { "Australia/Hobart",  1895,  9,  1,  0, 10, 44 },
    { "Australia/LHI",  1895,  1, 31, 23, 23, 40 },
    { "Australia/Lindeman",  1895,  1,  1,  0,  4,  4 },
    { "Australia/Lord_Howe",  1895,  1, 31, 23, 23, 40 },
    { "Australia/Melbourne",  1895,  2,  1,  0, 20,  8 },
    { "Australia/NSW",  1895,  1, 31, 23, 55,  8 },
    { "Australia/North",  1895,  2,  1,  0, 16, 40 },
    { "Australia/Perth",  1895, 12,  1,  0, 16, 36 },
    { "Australia/Queensland",  1894, 12, 31, 23, 47, 52 },
    { "Australia/South",  1895,  1, 31, 23, 45, 40 },
    { "Australia/Sydney",  1895,  1, 31, 23, 55,  8 },
    { "Australia/Tasmania",  1895,  9,  1,  0, 10, 44 },
    { "Australia/Victoria",  1895,  2,  1,  0, 20,  8 },
    { "Australia/West",  1895, 12,  1,  0, 16, 36 },
    { "Australia/Yancowinna",  1895,  2,  1,  0, 34, 12 },
    { "Brazil/Acre",  1913, 12, 31, 23, 31, 12 },
    { "Brazil/DeNoronha",  1914,  1,  1,  0,  9, 40 },
    { "Brazil/East",  1914,  1,  1,  0,  6, 28 },
    { "Brazil/West",  1914,  1,  1,  0,  0,  4 },
    { "Canada/Atlantic",  1902,  6, 15,  0, 14, 24 },
    { "Canada/Central",  1887,  7, 16,  0, 28, 36 },
    { "Canada/Eastern",  1895,  1,  1,  0, 17, 32 },
    { "Canada/Mountain",  1906,  9,  1,  0, 33, 52 },
    { "Canada/Newfoundland",  1884,  1,  1,  0,  0,  0 },
    { "Canada/Pacific",  1884,  1,  1,  0, 12, 28 },
    { "Canada/Saskatchewan",  1905,  8, 31, 23, 58, 36 },
    { "Canada/Yukon",  1900,  8, 20,  0,  0, 12 },
    { "Chile/Continental",  1890,  1,  1,  0,  0,  0 },
    { "Chile/EasterIsland",  1890,  1,  1,  0,  0,  0 },
    { "Europe/Amsterdam",  1880,  1,  1,  0,  0,  0 },
    { "Europe/Andorra",  1900, 12, 31, 23, 53, 56 },
    { "Europe/Astrakhan",  1924,  4, 30, 23, 47, 48 },
    { "Europe/Athens",  1895,  9, 14,  0,  0,  0 },
    { "Europe/Belfast",  1847, 12,  1,  0,  1, 15 },
    { "Europe/Belgrade",  1883, 12, 31, 23, 38,  0 },
    { "Europe/Berlin",  1893,  4,  1,  0,  6, 32 },
    { "Europe/Bratislava",  1850,  1,  1,  0,  0,  0 },
    { "Europe/Brussels",  1880,  1,  1,  0,  0,  0 },
    { "Europe/Bucharest",  1891, 10,  1,  0,  0,  0 },
    { "Europe/Budapest",  1890, 10, 31, 23, 43, 40 },
    { "Europe/Busingen",  1853,  7, 15, 23, 55, 38 },
    { "Europe/Chisinau",  1879, 12, 31, 23, 59, 40 },
    { "Europe/Copenhagen",  1893,  4,  1,  0,  6, 32 },
    { "Europe/Dublin",  1880,  8,  2,  0,  0,  0 },
    { "Europe/Gibraltar",  1880,  8,  2,  0, 21, 24 },
    { "Europe/Guernsey",  1847, 12,  1,  0,  1, 15 },
    { "Europe/Helsinki",  1878,  5, 31,  0,  0,  0 },
    { "Europe/Isle_of_Man",  1847, 12,  1,  0,  1, 15 },
    { "Europe/Istanbul",  1880,  1,  1,  0,  1,  4 },
    { "Europe/Jersey",  1847, 12,  1,  0,  1, 15 },
    { "Europe/Kaliningrad",  1893,  3, 31, 23, 38,  0 },
    { "Europe/Kiev",  1880,  1,  1,  0,  0,  0 },
    { "Europe/Kirov",  1919,  7,  1,  3,  0,  0 },
    { "Europe/Kyiv",  1880,  1,  1,  0,  0,  0 },
    { "Europe/Lisbon",  1912,  1,  1,  0,  0,  0 },
    { "Europe/Ljubljana",  1883, 12, 31, 23, 38,  0 },
    { "Europe/London",  1847, 12,  1,  0,  1, 15 },
    { "Europe/Luxembourg",  1880,  1,  1,  0,  0,  0 },
    { "Europe/Madrid",  1901,  1,  1,  0,  0,  0 },
    { "Europe/Malta",  1893, 11,  2,  0,  1, 56 },
    { "Europe/Mariehamn",  1878,  5, 31,  0,  0,  0 },
    { "Europe/Minsk",  1879, 12, 31, 23, 59, 44 },
    { "Europe/Monaco",  1891,  3, 16,  0,  0,  0 },
    { "Europe/Moscow",  1880,  1,  1,  0,  0,  0 },
    { "Europe/Nicosia",  1921, 11, 13, 23, 46, 32 },
    { "Europe/Oslo",  1893,  4,  1,  0,  6, 32 },
    { "Europe/Paris",  1891,  3, 16,  0,  0,  0 },
    { "Europe/Podgorica",  1883, 12, 31, 23, 38,  0 },
    { "Europe/Prague",  1850,  1,  1,  0,  0,  0 },
    { "Europe/Riga",  1880,  1,  1,  0,  0,  0 },
    { "Europe/Rome",  1866, 12, 12,  0,  0,  0 },
    { "Europe/Samara",  1919,  7,  1,  3,  0,  0 },
    { "Europe/San_Marino",  1866, 12, 12,  0,  0,  0 },
    { "Europe/Sarajevo",  1883, 12, 31, 23, 38,  0 },
    { "Europe/Saratov",  1919,  7,  1,  3,  0,  0 },
    { "Europe/Simferopol",  1879, 12, 31, 23, 59, 36 },
    { "Europe/Skopje",  1883, 12, 31, 23, 38,  0 },
    { "Europe/Sofia",  1880,  1,  1,  0, 23, 40 },
    { "Europe/Stockholm",  1893,  4,  1,  0,  6, 32 },
    { "Europe/Tallinn",  1880,  1,  1,  0,  0,  0 },
    { "Europe/Tirane",  1913, 12, 31, 23, 40, 40 },
    { "Europe/Tiraspol",  1879, 12, 31, 23, 59, 40 },
    { "Europe/Ulyanovsk",  1919,  7,  1,  3,  0,  0 },
    { "Europe/Uzhgorod",  1880,  1,  1,  0,  0,  0 },
    { "Europe/Vaduz",  1853,  7, 15, 23, 55, 38 },
    { "Europe/Vatican",  1866, 12, 12,  0,  0,  0 },
    { "Europe/Vienna",  1893,  3, 31, 23, 54, 39 },
    { "Europe/Vilnius",  1879, 12, 31, 23, 42, 44 },
    { "Europe/Volgograd",  1920,  1,  3,  0,  2, 20 },
    { "Europe/Warsaw",  1880,  1,  1,  0,  0,  0 },
    { "Europe/Zagreb",  1883, 12, 31, 23, 38,  0 },
    { "Europe/Zaporozhye",  1880,  1,  1,  0,  0,  0 },
    { "Europe/Zurich",  1853,  7, 15, 23, 55, 38 },
    { "Indian/Antananarivo",  1908,  5,  1,  0,  2, 44 },
    { "Indian/Chagos",  1907,  1,  1,  0, 10, 20 },
    { "Indian/Christmas",  1880,  1,  1,  0,  0,  0 },
    { "Indian/Cocos",  1880,  1,  1,  0,  0,  0 },
    { "Indian/Comoro",  1908,  5,  1,  0,  2, 44 },
    { "Indian/Kerguelen",  1880,  1,  1,  0,  0,  0 },
    { "Indian/Mahe",  1920,  1,  1,  0, 18, 48 },
    { "Indian/Maldives",  1880,  1,  1,  0,  0,  0 },
    { "Indian/Mauritius",  1907,  1,  1,  0, 10,  0 },
    { "Indian/Mayotte",  1908,  5,  1,  0,  2, 44 },
    { "Indian/Reunion",  1920,  1,  1,  0, 18, 48 },
    { "Mexico/BajaNorte",  1922,  1,  1,  0,  0,  0 },
    { "Mexico/BajaSur",  1922,  1,  1,  0,  0,  0 },
    { "Mexico/General",  1922,  1,  1,  0,  0,  0 },
    { "Pacific/Apia",  1910, 12, 31, 23, 56, 56 },
    { "Pacific/Auckland",  1868, 11,  1, 23, 50, 56 },
    { "Pacific/Bougainville",  1879, 12, 31, 23, 26, 16 },
    { "Pacific/Chatham",  1868, 11,  2,  0,  1, 12 },
    { "Pacific/Chuuk",  1879, 12, 31, 23, 59, 52 },
    { "Pacific/Easter",  1890,  1,  1,  0,  0,  0 },
    { "Pacific/Efate",  1912,  1, 12, 23, 46, 44 },
    { "Pacific/Fakaofo",  1901,  1,  1,  0, 24, 56 },
    { "Pacific/Fiji",  1915, 10, 26,  0,  4, 16 },
    { "Pacific/Funafuti",  1901,  1,  1,  0, 27, 56 },
    { "Pacific/Galapagos",  1931,  1,  1,  0, 58, 24 },
    { "Pacific/Gambier",  1912,  9, 30, 23, 59, 48 },
    { "Pacific/Guadalcanal",  1912, 10,  1,  0, 20, 12 },
    { "Pacific/Guam",  1901,  1,  1,  0, 21,  0 },
    { "Pacific/Honolulu",  1896,  1, 13, 12,  1, 26 },
    { "Pacific/Johnston",  1896,  1, 13, 12,  1, 26 },
    { "Pacific/Kiritimati",  1900, 12, 31, 23, 49, 20 },
    { "Pacific/Kosrae",  1901,  1,  1,  0,  8,  4 },
    { "Pacific/Kwajalein",  1900, 12, 31, 23, 50, 40 },
    { "Pacific/Majuro",  1901,  1,  1,  0, 27, 56 },
    { "Pacific/Marquesas",  1912,  9, 30, 23, 48,  0 },
    { "Pacific/Midway",  1911,  1,  1,  0, 22, 48 },
    { "Pacific/Nauru",  1921,  1, 15,  0, 22, 20 },
    { "Pacific/Niue",  1952, 10, 15, 23, 59, 40 },
    { "Pacific/Norfolk",  1901,  1,  1,  0,  0,  8 },
    { "Pacific/Noumea",  1912,  1, 12, 23, 54, 12 },
    { "Pacific/Pago_Pago",  1911,  1,  1,  0, 22, 48 },
    { "Pacific/Palau",  1901,  1,  1,  0,  2,  4 },
    { "Pacific/Pitcairn",  1901,  1,  1,  0, 10, 20 },
    { "Pacific/Pohnpei",  1912, 10,  1,  0, 20, 12 },
    { "Pacific/Ponape",  1912, 10,  1,  0, 20, 12 },
    { "Pacific/Port_Moresby",  1879, 12, 31, 23, 59, 52 },
    { "Pacific/Rarotonga",  1952, 10, 16,  0,  9,  4 },
    { "Pacific/Saipan",  1901,  1,  1,  0, 21,  0 },
    { "Pacific/Samoa",  1911,  1,  1,  0, 22, 48 },
    { "Pacific/Tahiti",  1912,  9, 30, 23, 58, 16 },
    { "Pacific/Tarawa",  1901,  1,  1,  0, 27, 56 },
    { "Pacific/Tongatapu",  1945,  9, 10,  0,  0, 48 },
    { "Pacific/Truk",  1879, 12, 31, 23, 59, 52 },
    { "Pacific/Wake",  1901,  1,  1,  0, 27, 56 },
    { "Pacific/Wallis",  1901,  1,  1,  0, 27, 56 },
    { "Pacific/Yap",  1879, 12, 31, 23, 59, 52 },
    { "US/Alaska",  1900,  8, 20, 11, 59, 36 },
    { "US/Aleutian",  1900,  8, 20, 12, 46, 38 },
    { "US/Arizona",  1883, 11, 18, 12,  0,  0 },
    { "US/Central",  1883, 11, 18, 12,  0,  0 },
    { "US/East-Indiana",  1883, 11, 18, 12,  0,  0 },
    { "US/Eastern",  1883, 11, 18, 12,  0,  0 },
    { "US/Hawaii",  1896,  1, 13, 12,  1, 26 },
    { "US/Indiana-Starke",  1883, 11, 18, 12,  0,  0 },
    { "US/Michigan",  1904, 12, 31, 23, 32, 11 },
    { "US/Mountain",  1883, 11, 18, 12,  0,  0 },
    { "US/Pacific",  1883, 11, 18, 12,  0,  0 },
    { "US/Samoa",  1911,  1,  1,  0, 22, 48 },
};

static int lmt_check(const struct cdata *cdata, const struct lmt *rule)
{
    if (cdata->year != rule->year)
        return cdata->year < rule->year;

    if (cdata->mon != rule->mon)
        return cdata->mon < rule->mon;

    if (cdata->mday != rule->mday)
        return cdata->mday < rule->mday;

    if (cdata->hour != rule->hour)
        return cdata->hour < rule->hour;

    if (cdata->min != rule->min)
        return cdata->min < rule->min;

    return cdata->sec < rule->sec;
}

static int is_lmt_date(const struct cdata *cdata)
{
    for (size_t i = 0; i < sizeof(dates) / sizeof(dates[0]); ++i)
    {
        if (strcmp(cdata->timezone, dates[i].timezone) == 0)
			return lmt_check(cdata, &dates[i]);
    }
    return 0;
}

void calculate_utc(struct cdata *cdata)
{
	struct tm tm_in = {0};
	tm_in.tm_year = cdata->year - 1900;
	tm_in.tm_mon = cdata->mon - 1;
	tm_in.tm_mday = cdata->mday;
	tm_in.tm_hour = cdata->hour;
	tm_in.tm_min = cdata->min;
	tm_in.tm_sec = cdata->sec;
	tm_in.tm_isdst = cdata->isdst;
	
	time_t t = mktime(&tm_in);
	
	if (is_lmt_date(cdata))
	{
		int offset = (int)lround(cdata->dlon * 240.0);
		t += (int)TM_GMTOFF(tm_in) - offset;
	}
	
	struct tm *tm_utc = gmtime(&t);
	
	cdata->utc_hour = tm_utc->tm_hour + tm_utc->tm_min / 60.0 +
	tm_utc->tm_sec / 3600.0;
	
	cdata->utc_year = tm_utc->tm_year + 1900;
	cdata->utc_mon = tm_utc->tm_mon + 1;
	cdata->utc_mday = tm_utc->tm_mday;
}

void cpt(struct cdata *cdata, struct tm *temp, time_t *t, int x)
{
	struct tm *result;
	if (!x || x == 2)
	{
		temp->tm_year = cdata->year - 1900;
		temp->tm_mon = cdata->mon - 1;
		temp->tm_mday = cdata->mday;
		temp->tm_hour = cdata->hour;
		temp->tm_min = cdata->min;
		temp->tm_sec = cdata->sec;
		temp->tm_isdst = cdata->isdst;
		
		*t = mktime(temp);
	}
	if (x)
	{	
		result = localtime(t);
		cdata->year = result->tm_year + 1900;
		cdata->mon = result->tm_mon + 1;
		cdata->mday = result->tm_mday;
		cdata->hour = result->tm_hour;
		cdata->min = result->tm_min;
		cdata->sec = result->tm_sec;
		cdata->isdst = result->tm_isdst;
		cdata->wday = result->tm_wday;
	}
}

static int speed_sign(double speed)
{
	if (speed > 0.0)
		return 1;
	if (speed < 0.0)
		return -1;
	return 0;
}

static void find_station(double jd_start, double initial_speed, int direction, int ipl, double *offset, double *jd, double *longitude)
{
	const double coarse = 2.0;
	const double fine = 0.1;
	
	double jd_ut = jd_start;
	int iflag = SEFLG_SWIEPH | SEFLG_SPEED;
	double xx[6];
	char serr[AS_MAXCH];
	double speed = initial_speed;
	
	int initial_sign = speed_sign(initial_speed);
	if (initial_sign == 0)
		return;
		
	while (speed_sign(speed) == initial_sign)
	{
		jd_ut += direction * coarse;
		swe_calc_ut(jd_ut, ipl, iflag, xx, serr);
		speed = xx[LONG_S];
	} 
		
	while (speed_sign(speed) != initial_sign)
	{
		jd_ut -= direction * fine;
		swe_calc_ut(jd_ut, ipl, iflag, xx, serr);
		speed = xx[LONG_S];
	} 
	
	*offset = jd_ut - jd_start;
	*jd = jd_ut;
	*longitude = xx[LONG];
}

void retro_station(double jd_ut, double *planet[])
{
	const int station = 7;
	
	for (int ipl = SE_MERCURY; ipl <= SE_PLUTO; ipl++)
	{
		if (planet[ipl][NEXT_JUL] - jd_ut > JUL_SEC ||
		planet[ipl][PREV_JUL] - jd_ut < -JUL_SEC)
		{
			planet[ipl][NEXT_S] = planet[ipl][NEXT_JUL] - jd_ut;
			planet[ipl][PREV_S] = planet[ipl][PREV_JUL] - jd_ut;
		}
		
		if (planet[ipl][NEXT_S] <= 0.0 || planet[ipl][PREV_S] >= 0.0)
			planet[ipl][RET_INIT] = 0;
		
		if (planet[ipl][RET_INIT] < 0.5)
		{
			find_station(jd_ut, planet[ipl][LONG_S], +1, ipl, &planet[ipl][NEXT_S], &planet[ipl][NEXT_JUL], &planet[ipl][NEXT_Z]);
			find_station(jd_ut, planet[ipl][LONG_S], -1, ipl, &planet[ipl][PREV_S], &planet[ipl][PREV_JUL], &planet[ipl][PREV_Z]);
			planet[ipl][RET_INIT] = 1;
		}
	
		const int is_retro = planet[ipl][LONG_S] <= 0.0;
		
		planet[ipl][RETRO] = is_retro ? 1.0 : 0.0;
		
		if (planet[ipl][NEXT_S] <= station)
			planet[ipl][STATION] = is_retro ? STATION_D : STATION_R;
		else
			planet[ipl][STATION] = 0.0;
	}
}

void eclipse(double jd_ut, double *luna_eclipse, double *sol_eclipse)
{
	int iflag = SEFLG_SWIEPH;
	double tret[10];
	double xx[6];
	char serr[AS_MAXCH];
	
	if (sol_eclipse[E_INIT] > 0)
	{
		sol_eclipse[EN_JUL] = sol_eclipse[EN_FJUL] - jd_ut;
		sol_eclipse[EP_JUL] = jd_ut - sol_eclipse[EP_FJUL];
		
		luna_eclipse[EN_JUL] = luna_eclipse[EN_FJUL] - jd_ut;
		luna_eclipse[EP_JUL] = jd_ut - luna_eclipse[EP_FJUL];
	}
	
	if (sol_eclipse[EN_JUL] < JUL_SEC || luna_eclipse[EN_JUL] < JUL_SEC ||
	sol_eclipse[EP_JUL] < JUL_SEC || luna_eclipse[EP_JUL] < JUL_SEC)
		sol_eclipse[E_INIT] = 0.0;
		
	if (sol_eclipse[E_INIT] < 0.5)
	{
		swe_sol_eclipse_when_glob(jd_ut, iflag, 0, tret, NEXT_E, serr);
		swe_calc_ut(tret[0], SE_SUN, iflag, xx, serr);
		sol_eclipse[EN_JUL] = tret[0] - jd_ut;
		sol_eclipse[EN_FJUL] = tret[0];
		sol_eclipse[EN_SIGN] = ((int)xx[LONG] / 30) + 1;
		
		swe_sol_eclipse_when_glob(jd_ut, iflag, 0, tret, PREV_E, serr);
		swe_calc_ut(tret[0], SE_SUN, iflag, xx, serr);
		sol_eclipse[EP_JUL] = jd_ut - tret[0];
		sol_eclipse[EP_FJUL] = tret[0];
		sol_eclipse[EP_SIGN] = ((int)xx[LONG] / 30) + 1;
		
		swe_lun_eclipse_when(jd_ut, iflag, 0, tret, NEXT_E, serr);
		swe_calc_ut(tret[0], SE_MOON, iflag, xx, serr);
		luna_eclipse[EN_JUL] = tret[0] - jd_ut;
		luna_eclipse[EN_FJUL] = tret[0];
		luna_eclipse[EN_SIGN] = ((int)xx[LONG] / 30) + 1;
		
		swe_lun_eclipse_when(jd_ut, iflag, 0, tret, PREV_E, serr);
		swe_calc_ut(tret[0], SE_MOON, iflag, xx, serr);
		luna_eclipse[EP_JUL] = jd_ut - tret[0];
		luna_eclipse[EP_FJUL] = tret[0];
		luna_eclipse[EP_SIGN] = ((int)xx[LONG] / 30) + 1;
		
		sol_eclipse[E_INIT] = 1.0;
	}
}

static void calc_return(struct cdata *cdata, double base_degree)
{
	struct tm temp = {0};
	time_t t = 0;

	cpt(cdata, &temp, &t, 0);
	
	int iflag = SEFLG_SWIEPH;
	double xx[6];
	char serr[AS_MAXCH];
	double diff;
	
	for(;;)
	{
		calculate_utc(cdata);
	
		double jd_ut = swe_julday(cdata->utc_year, cdata->utc_mon, 
		cdata->utc_mday, cdata->utc_hour, SE_GREG_CAL);
	
		swe_calc_ut(jd_ut, SE_SUN, iflag, xx, serr);
			
		diff = base_degree - xx[LONG];
		
		time_t step = 
			fabs(diff) > 2.0 ? 86400 :
			fabs(diff) > 0.1 ? 3600 :
			fabs(diff) > 0.002 ? 60 : 1;
			
		t+= (diff > 0.0 ? step : -step);
		
		cpt(cdata, &temp, &t, 1);
		
		if (fabs(diff) < JUL_SEC)
			break;
	}
}
	
void solar_return(struct cdata *cdata, struct pxx *pxx, struct ui *ui, double **planet, int **zodiac)
{
	double base_degree = pxx->dsun[LONG];
	
	int height = 5;
	int width = 30;
	
	int starty = (LINES- height) / 2;
	int startx = (COLS - width) / 2;
	
	WINDOW *sr_win = newwin(height, width, starty, startx);
	WINDOW *sr_subwin = derwin(sr_win, height - 2, width - 2, 0, 0);
	
	wbkgdset(sr_win, COLOR_PAIR(M_COLOR));
	
	cbreak();
	keypad(sr_win, TRUE);	
	
	FIELD *sr_field[2];
	sr_field[0] = new_field(1, 25, 2, 2, 0, 0);
	set_field_back(sr_field[0], COLOR_PAIR (M_COLOR) | A_UNDERLINE);
	field_opts_off(sr_field[0], O_STATIC);
	field_opts_off(sr_field[0], O_AUTOSKIP);
	set_field_type(sr_field[0], TYPE_INTEGER, 0, -12998, 16799);
	
	sr_field[1] = NULL;
	
	FORM *sr_form = new_form(sr_field);
	set_form_win(sr_form, sr_win);
	set_form_sub(sr_form, sr_subwin);
	
	post_form(sr_form);
	box(sr_win, 0, 0);
	mvwaddstr(sr_win, 1, 1, "-o--year?-o");
	
	set_current_field(sr_form, sr_field[0]);
	wrefresh(sr_win);
	pos_form_cursor(sr_form);
	
	int done = 0, ch = 0, c = 0;
	while(!done && (ch = wgetch(sr_win)))
	{
		switch (ch)
		{
			case '\n':
				form_driver(sr_form, REQ_VALIDATION);
				done = 1;
				break;
			case KEY_BACKSPACE:
				form_driver(sr_form, REQ_DEL_PREV);
				break;
			case KEY_LEFT:
				form_driver(sr_form, REQ_LEFT_CHAR);
				break;
			case KEY_RIGHT:
				form_driver(sr_form, REQ_RIGHT_CHAR);
				break;
			case 27:
				c = 1;
				done = 1;
				break;
			default:
				form_driver(sr_form, ch);
				break;
		}
		wrefresh(sr_win);
	}
	
	if (!c)
	{
		char *endptr = NULL;
		long iret;
		errno = 0;
		
		char *sr_year = field_buffer(sr_field[0], 0);
		int len = 0;
		
		field_info(sr_field[0], NULL, &len, NULL, NULL, NULL, NULL);
		
		while (len > 0 && sr_year[len - 1] == ' ')
			len--;
		sr_year[len] = '\0';
		
		iret = strtol(sr_year, &endptr, 10);
		if (errno != ERANGE)
			cdata->year = (int)iret;
		else
			cdata->year = 1970;
	}

	werase(sr_subwin);
	wrefresh(sr_subwin);
	werase(sr_win);
	wrefresh(sr_win);
	delwin(sr_subwin);
	delwin(sr_win);
				
	unpost_form(sr_form);
	set_form_fields(sr_form, NULL);
	for (int i = 0; i < 2; ++i)
		free_field(sr_field[i]);
	free_form(sr_form);
	
	calc_return(cdata, base_degree);
	
	new_chart(cdata, pxx, ui, planet, zodiac);
	doupdate();
}
