/*
 * sensord
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#ifndef __PHYSICAL_SENSOR_HANDLER_H__
#define __PHYSICAL_SENSOR_HANDLER_H__

#include <sensor_types.h>
#include <physical_sensor.h>
#include <unordered_map>

#include "sensor_handler.h"
#include "sensor_observer.h"

namespace sensor {

class physical_sensor_handler : public sensor_handler {
public:
	physical_sensor_handler(const sensor_info &info,
			sensor_device *device, int hal_id,
			physical_sensor *sensor);
	virtual ~physical_sensor_handler();

	/* functions for sensor device(HAL) */
	int get_hal_id(void);
	int get_poll_fd(void);
	int read_fd(std::vector<uint32_t> &hal_ids);
	int on_event(const sensor_data_t *data, int32_t len, int32_t remains);

	/* sensor interface */
	const sensor_info &get_sensor_info(void);

	int start(sensor_observer *ob);
	int stop(sensor_observer *ob);

	int set_interval(sensor_observer *ob, int32_t interval);
	int set_batch_latency(sensor_observer *ob, int32_t latency);
	int set_attribute(sensor_observer *ob, int32_t attr, int32_t value);
	int set_attribute(sensor_observer *ob, int32_t attr, const char *value, int len);
	int flush(sensor_observer *ob);
	int get_data(sensor_data_t **data, int *len);

private:
	int get_min_interval(void);
	int get_min_batch_latency(void);

	sensor_info m_info;
	sensor_device *m_device;
	physical_sensor *m_sensor;
	int m_hal_id;

	std::unordered_map<sensor_observer *, int> m_interval_map;
	std::unordered_map<sensor_observer *, int> m_batch_latency_map;
};

}

#endif /* __PHYSICAL_SENSOR_HANDLER_H__ */
