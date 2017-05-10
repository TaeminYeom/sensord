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

#include "sensor_handler.h"

#include <message.h>
#include <sensor_log.h>

using namespace sensor;

sensor_handler::sensor_handler(const sensor_info &info)
: m_info(info)
{
}

bool sensor_handler::has_observer(sensor_observer *ob)
{
	for (auto it = m_observers.begin(); it != m_observers.end(); ++it) {
		if ((*it) == ob)
			return true;
	}

	return false;
}

void sensor_handler::add_observer(sensor_observer *ob)
{
	ret_if(has_observer(ob));

	m_observers.push_back(ob);
}

void sensor_handler::remove_observer(sensor_observer *ob)
{
	m_observers.remove(ob);
}

int sensor_handler::notify(const char *uri, sensor_data_t *data, int len)
{
	if (observer_count() == 0)
		return OP_ERROR;

	ipc::message *msg;

	msg = new(std::nothrow) ipc::message((char *)data, len);
	retvm_if(!msg, OP_ERROR, "Failed to allocate memory");

	for (auto it = m_observers.begin(); it != m_observers.end(); ++it)
		(*it)->update(uri, msg);

	if (msg->ref_count() == 0)
		msg->unref();

	return OP_SUCCESS;
}

uint32_t sensor_handler::observer_count(void)
{
	return m_observers.size();
}
