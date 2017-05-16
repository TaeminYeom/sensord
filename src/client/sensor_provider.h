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

#ifndef __SENSOR_PROVIDER_H__
#define __SENSOR_PROVIDER_H__

#include <ipc_client.h>
#include <channel.h>
#include <channel_handler.h>
#include <event_loop.h>
#include <sensor_internal.h>
#include <sensor_info.h>
#include <sensor_types.h>
#include <map>
#include <atomic>

namespace sensor {

class sensor_provider {
public:
	sensor_provider(const char *uri);
	virtual ~sensor_provider();

	const char *get_uri(void);
	sensor_info *get_sensor_info(void);

	int connect(void);
	bool disconnect(void);
	void restore(void);

	void set_start_cb(sensord_provider_start_cb cb, void *user_data);
	void set_stop_cb(sensord_provider_stop_cb cb, void *user_data);
	void set_interval_cb(sensord_provider_interval_changed_cb cb, void *user_data);

	int publish(sensor_data_t *data, int len);

private:
	class channel_handler;

	bool init(const char *uri);
	void deinit(void);

	bool is_connected(void);

	int serialize(sensor_info *info, char **bytes);
	int send_sensor_info(sensor_info *info);

	sensor_info m_sensor;

	ipc::ipc_client *m_client;
	ipc::channel *m_channel;
	ipc::event_loop m_loop;
	channel_handler *m_handler;
	std::atomic<bool> m_connected;
};

}

#endif /* __SENSOR_PROVIDER_H__ */
