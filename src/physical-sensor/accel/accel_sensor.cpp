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

#include "accel_sensor.h"
#include <sensor_log.h>

#define URI_ACCEL "http://tizen.org/sensor/general/accelerometer"

accel_sensor::accel_sensor()
{
	_I("accel_sensor is created : %#x", this);
}

accel_sensor::~accel_sensor()
{
	_I("accel_sensor is destroyed : %#x", this);
}

std::string accel_sensor::get_uri(void)
{
	return URI_ACCEL;
}

physical_sensor *accel_sensor::clone(void) const
{
	return new accel_sensor(*this);
}

int accel_sensor::start(observer_h ob)
{
	return OP_DEFAULT;
}

int accel_sensor::stop(observer_h ob)
{
	return OP_DEFAULT;
}

int accel_sensor::set_interval(observer_h ob, uint32_t interval)
{
	return OP_DEFAULT;
}

int accel_sensor::set_batch_latency(observer_h ob, uint32_t latency)
{
	return OP_DEFAULT;
}

int accel_sensor::set_attribute(observer_h ob, int32_t attr, int32_t value)
{
	return OP_DEFAULT;
}

int accel_sensor::set_attribute(observer_h ob, int32_t attr, const char *value, int len)
{
	return OP_DEFAULT;
}

int accel_sensor::flush(observer_h ob)
{
	return OP_DEFAULT;
}

int accel_sensor::on_event(sensor_data_t *data, int32_t len, int32_t remains)
{
	return OP_DEFAULT;
}
