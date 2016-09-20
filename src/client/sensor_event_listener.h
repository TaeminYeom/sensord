/*
 * sensord
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
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

#ifndef _SENSOR_EVENT_LISTENER_H_
#define _SENSOR_EVENT_LISTENER_H_

#include <glib.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <csocket.h>
#include <string.h>
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <cmutex.h>
#include <poller.h>

#include <sensor_handle_info.h>
#include <sensor_client_info.h>
#include <command_channel.h>
#include <sensor_callback_deliverer.h>

typedef std::vector<unsigned int> handle_vector;
typedef std::vector<sensor_id_t> sensor_id_vector;
typedef std::unordered_map<int, sensor_handle_info> sensor_handle_info_map;
typedef std::unordered_map<sensor_id_t, command_channel*> sensor_command_channel_map;

typedef void (*hup_observer_t)(void);

class sensor_event_listener {
public:
	static sensor_event_listener& get_instance(void);

	void get_listening_sensors(sensor_id_vector &sensors);

	bool start_event_listener(void);
	void stop_event_listener(void);
	void clear(void);

	void set_hup_observer(hup_observer_t observer);

	void set_sensor_axis(int axis);
	void set_display_rotation(int rt);

private:
	enum thread_state {
		THREAD_STATE_START,
		THREAD_STATE_STOP,
		THREAD_STATE_TERMINATE,
	};
	typedef std::lock_guard<std::mutex> lock;
	typedef std::unique_lock<std::mutex> ulock;

	csocket m_event_socket;
	poller *m_poller;

	thread_state m_thread_state;
	std::mutex m_thread_mutex;
	std::condition_variable m_thread_cond;

	hup_observer_t m_hup_observer;

	sensor_client_info &m_client_info;

	sensor_callback_deliverer *m_cb_deliverer;

	/* WC1's rotation control */
	/* SENSORD_AXIS_DEVICE_ORIENTED, SENSORD_AXIS_DISPLAY_ORIENTED */
	int m_axis;
	int m_display_rotation;

	sensor_event_listener();
	~sensor_event_listener();

	bool create_event_channel(void);
	void close_event_channel(void);

	ssize_t sensor_event_poll(void *buffer, int buffer_len, struct epoll_event &event);

	void listen_events(void);
	void handle_events(void* event);

	client_callback_info* handle_calibration_cb(sensor_handle_info &handle_info, unsigned event_type, unsigned long long time, int accuracy);
	client_callback_info* get_callback_info(sensor_id_t sensor_id, const reg_event_info *event_info, std::shared_ptr<void> sensor_data);

	unsigned long long renew_event_id(void);

	void set_thread_state(thread_state state);

	/* WC1's sensor axis alignment */
	void align_sensor_axis(sensor_id_t sensor, sensor_data_t *data);

	bool start_deliverer(void);
	bool stop_deliverer(void);
};

#endif /* _SENSOR_EVENT_LISTENER_H_ */
