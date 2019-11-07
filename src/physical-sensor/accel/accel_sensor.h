/*
 * sensord
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#ifndef __ACCEL_SENSOR_H__
#define __ACCEL_SENSOR_H__

#include <physical_sensor.h>

class accel_sensor : public physical_sensor {
public:
	accel_sensor();
	~accel_sensor();

	std::string get_uri(void);
	physical_sensor *clone(void) const;

	int start(observer_h ob);
	int stop(observer_h ob);

	int set_interval(observer_h ob, uint32_t interval);
	int set_batch_latency(observer_h ob, uint32_t latency);
	int set_attribute(observer_h ob, int32_t attr, int32_t value);
	int set_attribute(observer_h ob, int32_t attr, const char *value, int len);
	int flush(observer_h ob);
	int on_event(sensor_data_t *data, int32_t len, int32_t remains);
};

#endif /* __ACCEL_SENSOR_H__ */

