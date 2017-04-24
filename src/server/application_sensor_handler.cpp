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

#include "application_sensor_handler.h"

#include <sensor_log.h>
#include <algorithm>

using namespace sensor;

application_sensor_handler::application_sensor_handler(const sensor_info &info)
: m_info(info)
{
}

application_sensor_handler::~application_sensor_handler()
{
}

int application_sensor_handler::post(sensor_data_t *data, int len)
{
	std::string uri = m_info.get_uri();
	return notify(uri.c_str(), data, len);
}

const sensor_info &application_sensor_handler::get_sensor_info(void)
{
	return m_info;
}

int application_sensor_handler::start(sensor_observer *ob)
{
	add_observer(ob);

	return OP_SUCCESS;
}

int application_sensor_handler::stop(sensor_observer *ob)
{
	remove_observer(ob);

	return OP_SUCCESS;
}

int application_sensor_handler::set_interval(sensor_observer *ob, int32_t interval)
{
	return OP_SUCCESS;
}

int application_sensor_handler::set_batch_latency(sensor_observer *ob, int32_t latency)
{
	return OP_SUCCESS;
}

int application_sensor_handler::set_attribute(sensor_observer *ob, int32_t attr, int32_t value)
{
	return OP_SUCCESS;
}

int application_sensor_handler::set_attribute(sensor_observer *ob, int32_t attr, const char *value, int len)
{
	return OP_SUCCESS;
}

int application_sensor_handler::flush(sensor_observer *ob)
{
	return OP_SUCCESS;
}

int application_sensor_handler::get_data(sensor_data_t **data, int *len)
{
	return OP_SUCCESS;
}
