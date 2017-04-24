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

#ifndef __SENSOR_PROVIDER_HANDLER__
#define __SENSOR_PROVIDER_HANDLER__

#include <sensor_internal.h>
#include <channel_handler.h>

namespace sensor {

class sensor_provider;

class sensor_provider_handler : public ipc::channel_handler
{
public:
	sensor_provider_handler(sensor_provider *provider);

	void connected(ipc::channel *ch);
	void disconnected(ipc::channel *ch);
	void read(ipc::channel *ch, ipc::message &msg);

	void read_complete(ipc::channel *ch);
	void error_caught(ipc::channel *ch, int error);

	void set_start_cb(sensord_provider_start_cb cb, void *user_data);
	void set_stop_cb(sensord_provider_stop_cb cb, void *user_data);
	void set_interval_cb(sensord_provider_set_interval_cb cb, void *user_data);

private:
	sensor_provider *m_provider;

	sensord_provider_start_cb m_start_cb;
	sensord_provider_stop_cb m_stop_cb;
	sensord_provider_set_interval_cb m_set_interval_cb;

	void *m_start_user_data;
	void *m_stop_user_data;
	void *m_set_interval_user_data;
};

}

#endif /* __SENSOR_PROVIDER_HANDLER__ */
