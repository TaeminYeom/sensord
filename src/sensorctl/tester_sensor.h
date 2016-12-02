/*
 * sensorctl
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#pragma once // _SENSOR_TESTER_H_

#include "tester.h"

class tester_sensor : public tester_interface {
public:
	tester_sensor() {}
	virtual ~tester_sensor() {}

	virtual bool init(void);
	virtual bool test(sensor_type_t type, int option_count, char *options[]);
private:
	void test_sensor(sensor_type_t type, int interval, int latency, int cnt_test, int cnt_event);
	static void test_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data);
};
