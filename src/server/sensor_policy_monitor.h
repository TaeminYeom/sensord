/*
 * sensord
 *
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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

#ifndef __SENSOR_POLICY_MONITOR_H__
#define __SENSOR_POLICY_MONITOR_H__

#include <channel.h>
#include <message.h>
#include <vconf.h>
#include <set>

#include "sensor_policy_listener.h"

namespace sensor {

class sensor_policy_monitor {
public:
	~sensor_policy_monitor();

	static sensor_policy_monitor& get_instance(void);

	void add_listener(sensor_policy_listener *listener);
	void remove_listener(sensor_policy_listener *listener);

	void on_policy_changed(int policy, int value);

private:
	sensor_policy_monitor();

	std::set<sensor_policy_listener *> m_listeners;
};

}

#endif /* __SENSOR_POLICY_MONITOR_H__ */
