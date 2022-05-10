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

#ifndef __SENSOR_EVENT_HANDLER__
#define __SENSOR_EVENT_HANDLER__

#include <event_handler.h>
#include <set>

#include "physical_sensor_handler.h"

namespace sensor {

class sensor_event_handler : public ipc::event_handler
{
public:
	sensor_event_handler();

	void add_sensor(physical_sensor_handler *sensor);
	void remove_sensor(physical_sensor_handler *sensor);

	bool handle(int fd, ipc::event_condition condition, void **data);

private:
	std::set<physical_sensor_handler *> m_sensors;
};

}

#endif /* __SENSOR_EVENT_DISPATCHER__ */
