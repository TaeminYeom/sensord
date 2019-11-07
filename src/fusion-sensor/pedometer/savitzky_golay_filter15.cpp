/*
 *  Copyright (c) 2016-2017 Samsung Electronics Co., Ltd.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "savitzky_golay_filter15.h"

#include <memory>
#include <stdlib.h>

/* length of filter. changing it requires changing coef_array! */
#define ARRAY_SIZE 15

/* sum of numerators of elements in coef_array. */
#define SUM_COEF 1105.0

/* array with coefficients for savitzky-golay filter. must be length of n! */
static double coef_array[] = {
		-78 / SUM_COEF, -13 / SUM_COEF, 42 / SUM_COEF, 87 / SUM_COEF,
		122 / SUM_COEF, 147 / SUM_COEF, 162 / SUM_COEF, 167 / SUM_COEF,
		162 / SUM_COEF, 147 / SUM_COEF, 122 / SUM_COEF, 87 / SUM_COEF,
		42 / SUM_COEF, -13 / SUM_COEF, -78 / SUM_COEF };

savitzky_golay_filter15::savitzky_golay_filter15()
: m_empty(true)
{
	m_array = (double *)calloc(ARRAY_SIZE, sizeof(double));
}

savitzky_golay_filter15::~savitzky_golay_filter15()
{
	if (m_array == NULL)
		return;

	free(m_array);
	m_array = NULL;
}

double savitzky_golay_filter15::filter(double value)
{
	if (m_empty) {
		for (int i = 0; i < ARRAY_SIZE; i++)
			m_array[i] = value;
		m_empty = false;
		return value;
	}

	for (int i = 1; i < ARRAY_SIZE; i++)
		m_array[i - 1] = m_array[i];
	m_array[ARRAY_SIZE - 1] = value;

	double avrg = 0;
	for (int i = 0; i < ARRAY_SIZE; i++)
		avrg += m_array[i] * coef_array[i];
	return avrg;
}

void savitzky_golay_filter15::reset(void)
{
	m_empty = true;
}
