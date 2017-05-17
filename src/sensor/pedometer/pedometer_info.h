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

#ifndef __PEDOMETER_INFO_H__
#define __PEDOMETER_INFO_H__

#include "common.h"

/************************************************************************
 * stores information about pedometer event detected.
 */
class pedometer_info {
public:
	/** timestamp this event was detected in ns. */
	timestamp_t timestamp;

	/** is step detected. */
	bool is_step_detected;

	/** step count from scanner start. */
	long long step_count;

	/** step length in meters. */
	double step_length;

	/** total length of all steps detected from scanner start in meters. */
	double total_step_length;

	/** current mean speed in m/s. */
	double step_speed;
};

#endif /* __PEDOMETER_INFO_H__ */
