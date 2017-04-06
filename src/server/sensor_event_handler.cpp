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

#include "sensor_event_handler.h"

#include <sensor_log.h>
#include <sensor_utils.h>
#include <algorithm>

using namespace sensor;

static std::vector<uint32_t> ids;

sensor_event_handler::sensor_event_handler(physical_sensor_handler *sensor)
: m_sensor(sensor)
{
}

bool sensor_event_handler::handle(int fd, ipc::event_condition condition)
{
	sensor_info info;
	sensor_data_t *data;
	int length = 0;
	int remains = 1;

	if (m_sensor->read_fd(ids) < 0)
		return true;

	auto result = std::find(std::begin(ids), std::end(ids), m_sensor->get_hal_id());

	if (result == std::end(ids))
		return true;

	while (remains > 0) {
		remains = m_sensor->get_data(&data, &length);
		if (remains < 0) {
			_E("Failed to get sensor data");
			break;
		}

		if (m_sensor->on_event(data, length, remains) < 0) {
			free(data);
			continue;
		}

		info = m_sensor->get_sensor_info();

		//_I("[Data] allocate %p", data);
		if (m_sensor->notify(info.get_type_uri().c_str(), data, length) < 0) {
			free(data);
		}
		info.clear();
	}

	return true;
}
