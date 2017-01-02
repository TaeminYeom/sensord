/*
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

#ifndef _SENSOR_HAL_H_
#define _SENSOR_HAL_H_

#include <stdint.h>
#include <stdbool.h>
#include "sensor_hal_types.h"

/*
 * Create devices
 */
typedef void *sensor_device_t;
typedef int (*create_t)(sensor_device_t **devices);

/*
 * Sensor device interface
 * 1 device must be abstracted from 1 device event node
 */
class sensor_device {
public:
	virtual ~sensor_device() {}

	uint32_t get_hal_version(void)
	{
		return SENSOR_HAL_VERSION(1, 0);
	}

	virtual int get_poll_fd(void) = 0;
	virtual int get_sensors(const sensor_info_t **sensors) = 0;

	virtual bool enable(uint32_t id) = 0;
	virtual bool disable(uint32_t id) = 0;

	virtual int read_fd(uint32_t **ids) = 0;
	virtual int get_data(uint32_t id, sensor_data_t **data, int *length) = 0;

	virtual bool set_interval(uint32_t id, unsigned long val)
	{
		return true;
	}
	virtual bool set_batch_latency(uint32_t id, unsigned long val)
	{
		return true;
	}
	virtual bool set_attribute_int(uint32_t id, int32_t attribute, int32_t value)
	{
		return true;
	}
	virtual bool set_attribute_str(uint32_t id, int32_t attribute, char *value, int len)
	{
		return true;
	}
	virtual bool flush(uint32_t id)
	{
		return true;
	}
};

#endif /* _SENSOR_HAL_H_ */
