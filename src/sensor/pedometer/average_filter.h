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

#ifndef __AVERAGE_FILTER_H__
#define __AVERAGE_FILTER_H__

#include "timestamp.h"

class average_filter {
public:

	average_filter(int size);

	~average_filter();

	/************************************************************************
	 * Filters input data.
	 */
	double filter(double value);

	/************************************************************************
	 * Resets average filter to initial state.
	 */
	void reset(void);

private:
	/************************************************************************
	 * Average filter state.
	 */
	double *m_array;
	int m_size;
	int m_index;
	bool m_ready;
};

#endif /* __AVERAGE_FILTER_H__ */
