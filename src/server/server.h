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

#ifndef __SERVER_H__
#define __SERVER_H__

#include <event_loop.h>
#include <ipc_server.h>
#include <sensor_manager.h>
#include <server_channel_handler.h>
#include <physical_sensor.h>
#include <atomic>

namespace sensor {

class server {
public:
	static void run(void);
	static void stop(void);

private:
	static server &instance(void);

	static ipc::event_loop m_loop;
	static std::atomic<bool> is_running;

	server();
	~server();

	bool init(void);
	void deinit(void);

	void init_calibration(void);
	void init_server(void);
	void init_termination(void);

	ipc::ipc_server *m_server;
	sensor_manager *m_manager;
	server_channel_handler *m_handler;
};

}

#endif /* __SERVER_H__ */
