/*
 * sensord
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#ifndef _COMMAND_WORKER_H_
#define _COMMAND_WORKER_H_

#include <sensor_common.h>
#include <command_common.h>
#include <worker_thread.h>
#include <client_info_manager.h>
#include <sensor_event_dispatcher.h>
#include <sensor_base.h>
#include <map>

typedef std::multimap<int, raw_data_t> sensor_raw_data_map;

class command_worker {
private:
	typedef bool (command_worker::*cmd_handler_t)(void *payload);

	int m_client_id;
	int m_permission;
	csocket m_socket;
	worker_thread m_worker;
	sensor_base *m_module;
	sensor_id_t m_sensor_id;
	static cmd_handler_t m_cmd_handlers[CMD_CNT];
	static cpacket m_sensor_list;
	static sensor_raw_data_map m_sensor_raw_data_map;

	static void init_cmd_handlers(void);
	static size_t create_sensor_raw_list(int client_perms, std::vector<raw_data_t *> &raw_list);
	static void get_sensor_list(int permissions, cpacket &sensor_list);

	static bool working(void *ctx);
	static bool stopped(void *ctx);

	bool dispatch_command(int cmd, void *payload);

	bool send_cmd_done(long value);
	bool send_cmd_get_id_done(int client_id);
	bool send_cmd_get_data_done(int state, sensor_data_t *data);
	bool send_cmd_get_sensor_list_done(void);

	bool cmd_get_id(void *payload);
	bool cmd_get_sensor_list(void *payload);
	bool cmd_hello(void *payload);
	bool cmd_byebye(void *payload);
	bool cmd_get_value(void *payload);
	bool cmd_start(void *payload);
	bool cmd_stop(void *payload);
	bool cmd_register_event(void *payload);
	bool cmd_unregister_event(void *payload);
	bool cmd_set_batch(void *payload);
	bool cmd_unset_batch(void *payload);
	bool cmd_set_pause_policy(void *payload);
	bool cmd_get_data(void *payload);
	bool cmd_set_attribute_int(void *payload);
	bool cmd_set_attribute_str(void *payload);
	bool cmd_flush(void *payload);

	void get_info(std::string &info);

	int get_permission(void);
	bool is_permission_allowed(void);

	static client_info_manager& get_client_info_manager(void);
	static sensor_event_dispatcher& get_event_dispathcher(void);

protected:
	static cmutex m_shared_mutex;

public:
	command_worker(const csocket& socket);
	virtual ~command_worker();

	bool start(void);
};

#endif /* _COMMAND_WORKER_H_ */
