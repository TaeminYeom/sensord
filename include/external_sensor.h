/*
 * sensord
 *
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

#ifndef __EXTERNAL_SENSOR_H__
#define __EXTERNAL_SENSOR_H__

#include <stdint.h>
#include <stdbool.h>
#include <sensor_types.h>
#include <string>

#ifndef EXTERNAL_SENSOR_VERSION
#define EXTERNAL_SENSOR_VERSION(maj, min) \
		((((maj) & 0xFFFF) << 24) | ((min) & 0xFFFF))
#endif

#ifndef OP_SUCCESS
#define OP_SUCCESS 0
#endif
#ifndef OP_ERROR
#define OP_ERROR   -1
#endif
#ifndef OP_DEFAULT
#define OP_DEFAULT 1
#endif

/*
 * Create sensor
 */
typedef void *external_sensor_t;
typedef int (*create_external_t)(external_sensor_t **sensors);

typedef void *observer_h;

/*
 * Sensor event notifier
 * External Sensor has to call notify() function if data is ready.
 */
class sensor_notifier {
public:
	virtual ~sensor_notifier() {}

	virtual int notify(void) = 0;
};

/*
 * Sensor interface
 */
class external_sensor {
public:
	virtual ~external_sensor() {}

	inline uint32_t get_version(void) { return EXTERNAL_SENSOR_VERSION(1, 0); }

	virtual int get_sensor_info(const sensor_info2_t **info) = 0;
	virtual int set_notifier(sensor_notifier *notifier) = 0;
	virtual int get_data(sensor_data_t **data, int *len) = 0;

	virtual int start(observer_h ob)
	{
		return OP_DEFAULT;
	}
;
	virtual int stop(observer_h ob)
	{
		return OP_DEFAULT;
	}

	virtual int set_interval(observer_h ob, int32_t &interval)
	{
		return OP_DEFAULT;
	}

	virtual int set_batch_latency(observer_h ob, int32_t &latency)
	{
		return OP_DEFAULT;
	}

	virtual int set_attribute(observer_h ob, int32_t attr, int32_t value)
	{
		return OP_DEFAULT;
	}

	virtual int set_attribute(observer_h ob, int32_t attr, const char *value, int len)
	{
		return OP_DEFAULT;
	}

	virtual int flush(observer_h ob)
	{
		return OP_DEFAULT;
	}
};

#endif /* __EXTERNAL_SENSOR_H__ */
