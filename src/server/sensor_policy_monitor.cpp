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

#include "sensor_policy_monitor.h"

#include <sensor_log.h>
#include <sensor_types.h>

#ifndef VCONFKEY_SETAPPL_PSMODE
#define VCONFKEY_SETAPPL_PSMODE "db/setting/psmode"
#endif

using namespace sensor;

static int get_power_save_state(void)
{
	int ret;
	int state = 0;
	int pm_state, ps_state;

	ret = vconf_get_int(VCONFKEY_PM_STATE, &pm_state);

	if (!ret && pm_state == VCONFKEY_PM_STATE_LCDOFF)
		state |= SENSORD_PAUSE_ON_DISPLAY_OFF;

	ret = vconf_get_int(VCONFKEY_SETAPPL_PSMODE, &ps_state);

	if (!ret && ps_state != SETTING_PSMODE_NORMAL)
		state |= SENSORD_PAUSE_ON_POWERSAVE_MODE;

	return state;
}

static void power_save_state_cb(keynode_t *node, void *data)
{
	int power_save_state = get_power_save_state();

	sensor_policy_monitor::get_instance().on_policy_changed(SENSORD_ATTRIBUTE_PAUSE_POLICY, power_save_state);
}

sensor_policy_monitor& sensor_policy_monitor::get_instance(void)
{
	static sensor_policy_monitor mon;
	return mon;
}

sensor_policy_monitor::sensor_policy_monitor()
{
	vconf_notify_key_changed(VCONFKEY_PM_STATE, power_save_state_cb, NULL);
	vconf_notify_key_changed(VCONFKEY_SETAPPL_PSMODE, power_save_state_cb, NULL);
}

sensor_policy_monitor::~sensor_policy_monitor()
{
	vconf_ignore_key_changed(VCONFKEY_PM_STATE, power_save_state_cb);
	vconf_ignore_key_changed(VCONFKEY_SETAPPL_PSMODE, power_save_state_cb);
}

void sensor_policy_monitor::add_listener(sensor_policy_listener *listener)
{
	ret_if(!listener);

	m_listeners.insert(listener);
}

void sensor_policy_monitor::remove_listener(sensor_policy_listener *listener)
{
	ret_if(!listener);

	m_listeners.erase(listener);
}

void sensor_policy_monitor::on_policy_changed(int policy, int value)
{
	auto it = m_listeners.begin();

	while (it != m_listeners.end()) {
		(*it)->on_policy_changed(policy, value);
		++it;
	}
}
