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

#ifndef __STEP_EVENT_H__
#define __STEP_EVENT_H__

#include "common.h"

class step_event {
public:
	step_event()
	: m_timestamp(UNKNOWN_TIMESTAMP)
	, m_step_length(0)
	{}

	timestamp_t m_timestamp;
	double m_step_length;
};

#endif /* __STEP_EVENT_H__ */
