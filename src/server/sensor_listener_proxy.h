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

#ifndef __SENSOR_LISTENER_PROXY_H__
#define __SENSOR_LISTENER_PROXY_H__

#include <channel.h>
#include <message.h>

#include "sensor_manager.h"
#include "sensor_observer.h"
#include "sensor_policy_listener.h"

namespace sensor {

class sensor_listener_proxy : public sensor_observer, sensor_policy_listener {
public:
	sensor_listener_proxy(uint32_t id,
			std::string uri, sensor_manager *manager, ipc::channel *ch);
	~sensor_listener_proxy();

	uint32_t get_id(void);

	/* sensor observer */
	int update(const char *uri, std::shared_ptr<ipc::message> msg);
	int on_attribute_changed(std::shared_ptr<ipc::message> msg);

	int start(bool policy = false);
	int stop(bool policy = false);

	int set_interval(int32_t interval);
	int get_interval(int32_t& interval);
	int set_max_batch_latency(int32_t max_batch_latency);
	int get_max_batch_latency(int32_t& max_batch_latency);
	int delete_batch_latency(void);
	int set_passive_mode(bool passive);
	int set_attribute(int32_t attribute, int32_t value);
	int get_attribute(int32_t attribute, int32_t *value);
	int set_attribute(int32_t attribute, const char *value, int len);
	int get_attribute(int32_t attribute, char **value, int *len);
	int flush(void);
	int get_data(sensor_data_t **data, int *len);
	std::string get_required_privileges(void);

	/* sensor_policy_listener interface */
	void on_policy_changed(int policy, int value);
	bool notify_attribute_changed(int32_t attribute, int32_t value);
	bool notify_attribute_changed(int32_t attribute, const char *value, int len);
	bool need_to_notify_attribute_changed();
	void set_need_to_notify_attribute_changed(bool value);

private:
	void update_event(std::shared_ptr<ipc::message> msg);
	void update_accuracy(std::shared_ptr<ipc::message> msg);
	void apply_sensor_handler_need_to_notify_attribute_changed(sensor_handler* handler);

	uint32_t m_id;
	std::string m_uri;

	sensor_manager *m_manager;
	ipc::channel *m_ch;

	bool m_started;
	bool m_passive;
	int32_t m_pause_policy;
	int32_t m_axis_orientation;
	int32_t m_last_accuracy;
	bool m_need_to_notify_attribute_changed;
};

}

#endif /* __SENSOR_LISTENER_PROXY_H__ */
