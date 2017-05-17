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

#include "average_filter.h"

#include <stdlib.h>

static double mean(double *array, int size)
{
	double avrg = 0;

	for (int i = 0; i < size; ++i)
		avrg = avrg + array[i];

	return avrg / size;
}

average_filter::average_filter(int sz)
: m_size(sz)
, m_index(0)
, m_ready(false)
{
	m_array = (double *)calloc(sz, sizeof(double));
}

average_filter::~average_filter()
{
	if (m_array == NULL)
		return;

	free(m_array);
	m_array = NULL;
	m_size = 0;
}

double average_filter::filter(double value)
{
	m_array[m_index++] = value;

	if (m_index >= m_size) {
		m_ready = true;
		m_index = 0;
	}
	return mean(m_array, (m_ready ? m_size : m_index));
}

void average_filter::reset(void)
{
	m_index = 0;
	m_ready = false;
}
