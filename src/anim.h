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

#include <panel.h>

#define NEW_CHART_ARG() cdata, pxx, ui, \
	cusp, sign_cusp, planet, zodiac, \
	luna_eclipse, sol_eclipse 
	
#define NEW_CHART_PARAM() struct cdata *cdata, struct pxx *pxx, struct ui *ui, \
double cusp[], double sign_cusp[], double *planet[], int *zodiac[], \
double *luna_eclipse, double *sol_eclipse \

void cur_chart_data(WINDOW *win, struct cdata *cdata);

void new_chart(NEW_CHART_PARAM());

void animate_chart(NEW_CHART_PARAM());

void realtime_chart(NEW_CHART_PARAM());

void solar_return(NEW_CHART_PARAM());

void transit(NEW_CHART_PARAM(), struct cdata **c, struct pxx **p);
