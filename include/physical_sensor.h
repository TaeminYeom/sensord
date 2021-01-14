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

#ifndef __PHYSICAL_SENSOR_H__
#define __PHYSICAL_SENSOR_H__

#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <sensor_types.h>

#include <hal/hal-sensor-interface.h>

#ifndef SENSOR_VERSION
#define PHYSICAL_SENSOR_VERSION(maj, min) \
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
typedef void *physical_sensor_t;
typedef int (*create_physical_t)(physical_sensor_t **sensors);

typedef void *observer_h;

/*
 * Sensor interface
 */
class physical_sensor {
public:
	virtual ~physical_sensor() {}

	inline uint32_t get_version(void) { return PHYSICAL_SENSOR_VERSION(1, 0); }

	virtual std::string get_uri(void) { return ""; }
	virtual std::string get_privilege(void) { return ""; }

	virtual physical_sensor *clone(void) const = 0;
	virtual void set_device(sensor_device *device) { return; }

	virtual int start(observer_h ob)
	{
		return OP_DEFAULT;
	}

	virtual int stop(observer_h ob)
	{
		return OP_DEFAULT;
	}

	virtual int set_interval(observer_h ob, uint32_t interval)
	{
		return OP_DEFAULT;
	}

	virtual int set_batch_latency(observer_h ob, uint32_t latency)
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

	virtual int on_event(sensor_data_t *data, int32_t len, int32_t remains)
	{
		return OP_DEFAULT;
	}
};

#endif /* __PHYSICAL_SENSOR_H__ */
