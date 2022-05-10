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

sensor_event_handler::sensor_event_handler()
{
}

void sensor_event_handler::add_sensor(physical_sensor_handler *sensor)
{
	ret_if(!sensor);

	m_sensors.insert(sensor);
}

void sensor_event_handler::remove_sensor(physical_sensor_handler *sensor)
{
	ret_if(!sensor);

	m_sensors.erase(sensor);
}

bool sensor_event_handler::handle(int fd, ipc::event_condition condition, void **data)
{
	sensor_info info;
	sensor_data_t *sensor_data;
	physical_sensor_handler *sensor;
	int length = 0;
	int remains;

	retv_if(m_sensors.empty(), false);

	ids.clear();

	auto it = m_sensors.begin();

	/* sensors using the same fd share read_fd in common.
	 * so just call read_fd on the first sensor */
	if ((*it)->read_fd(ids) < 0)
		return true;

	for (; it != m_sensors.end(); ++it) {
		remains = 1;
		sensor = *it;

		/* check whether the id of this sensor is in id list(parameter) or not */
		auto result = std::find(std::begin(ids), std::end(ids), sensor->get_hal_id());
		if (result == std::end(ids))
			continue;

		while (remains > 0) {
			remains = sensor->get_data(&sensor_data, &length);
			if (remains < 0) {
				_E("Failed to get sensor data");
				break;
			}

			if (sensor->on_event(sensor_data, length, remains) < 0) {
				free(sensor_data);
				continue;
			}

			info = sensor->get_sensor_info();

			//_I("[Data] allocate %p", sensor_data);
			if (sensor->notify(info.get_uri().c_str(), sensor_data, length) < 0) {
				free(sensor_data);
			}
			info.clear();
		}
	}

	return true;
}
