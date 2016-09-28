/*
 * sensord
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#ifndef _SENSOR_CALLBACK_DELIVERER_H_
#define _SENSOR_CALLBACK_DELIVERER_H_

#include <glib.h>
#include <client_common.h>

#include <thread>
#include <atomic>

class sensor_callback_deliverer {
public:
	sensor_callback_deliverer();
	~sensor_callback_deliverer();

	bool start(void);
	bool stop(void);
	bool is_running(void);

	bool push(client_callback_info *ci);

private:
	static gboolean callback_dispatcher(gpointer data);

	std::atomic_bool m_running;

	GAsyncQueue *m_callbacks;
	std::thread *m_deliverer;

	void run(void);
	bool terminate(void);

	void deliver_to_main_loop(client_callback_info *ci);
};

#endif /* _SENSOR_CALLBACK_DELIVERER_H_ */
