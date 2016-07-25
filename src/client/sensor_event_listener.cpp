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

#include <sensor_event_listener.h>
#include <client_common.h>
#include <sensor_info_manager.h>

#include <thread>
#include <chrono>
#include <vector>

#include <sensor_types.h>

/* TODO: this macro should be adjusted(4224 = 4096(data) + 128(header)) */
#define EVENT_BUFFER_SIZE sizeof(sensor_event_t)

using std::thread;
using std::pair;
using std::vector;

struct free_data {
	void operator()(void *data) {
		free(data);
	}
};

sensor_event_listener::sensor_event_listener()
: m_poller(NULL)
, m_thread_state(THREAD_STATE_TERMINATE)
, m_hup_observer(NULL)
, m_client_info(sensor_client_info::get_instance())
, m_axis(SENSORD_AXIS_DISPLAY_ORIENTED)
, m_display_rotation(AUTO_ROTATION_DEGREE_UNKNOWN)
{
}

sensor_event_listener::~sensor_event_listener()
{
	stop_event_listener();
}

sensor_event_listener& sensor_event_listener::get_instance(void)
{
	static sensor_event_listener inst;
	return inst;
}

client_callback_info* sensor_event_listener::handle_calibration_cb(sensor_handle_info &handle_info, unsigned event_type, unsigned long long time, int accuracy)
{
	unsigned int cal_event_type = get_calibration_event_type(event_type);
	reg_event_info *event_info = NULL;
	reg_event_info *cal_event_info = NULL;
	client_callback_info* cal_callback_info = NULL;

	if (!cal_event_type)
		return NULL;

	cal_event_info = handle_info.get_reg_event_info(cal_event_type);
	if ((accuracy == SENSOR_ACCURACY_BAD) && !handle_info.m_bad_accuracy &&	cal_event_info) {
		cal_event_info->m_previous_event_time = time;

		event_info = handle_info.get_reg_event_info(event_type);
		if (!event_info)
			return NULL;

		sensor_data_t *cal_data = (sensor_data_t *)malloc(sizeof(sensor_data_t));
		retvm_if(!cal_data, NULL, "Failed to allocate memory");

		cal_data->accuracy = accuracy;
		cal_data->timestamp = time;
		cal_data->values[0] = accuracy;
		cal_data->value_count = 1;
		std::shared_ptr<void> cal_sensor_data(cal_data, free_data());

		cal_callback_info = get_callback_info(handle_info.m_sensor_id, cal_event_info, cal_sensor_data);

		m_client_info.set_bad_accuracy(handle_info.m_handle, true);

		print_event_occurrence_log(handle_info);
	}

	if ((accuracy != SENSOR_ACCURACY_BAD) && handle_info.m_bad_accuracy)
		m_client_info.set_bad_accuracy(handle_info.m_handle, false);

	return cal_callback_info;
}

void sensor_event_listener::handle_events(void* event)
{
	unsigned long long cur_time;
	reg_event_info *event_info = NULL;
	sensor_id_t sensor_id;
	sensor_handle_info_map handles_info;

	int accuracy = SENSOR_ACCURACY_GOOD;

	unsigned int event_type = *((unsigned int *)(event));

	client_callback_info* callback_info = NULL;
	vector<client_callback_info *> client_callback_infos;

	sensor_event_t *sensor_event = (sensor_event_t *)event;
	sensor_id = sensor_event->sensor_id;
	cur_time = sensor_event->data->timestamp;
	accuracy = sensor_event->data->accuracy;
	std::shared_ptr<void> sensor_data(sensor_event->data, free_data());

	{	/* scope for the lock */
		m_client_info.get_all_handle_info(handles_info);

		for (auto it_handle = handles_info.begin(); it_handle != handles_info.end(); ++it_handle) {
			sensor_handle_info &sensor_handle_info = it_handle->second;

			event_info = sensor_handle_info.get_reg_event_info(event_type);
			if ((sensor_handle_info.m_sensor_id != sensor_id) ||
			    !sensor_handle_info.is_started() ||
			    !event_info)
				continue;

			if (event_info->m_fired)
				continue;

			event_info->m_previous_event_time = cur_time;

			client_callback_info* cal_callback_info = handle_calibration_cb(sensor_handle_info, event_type, cur_time, accuracy);

			if (cal_callback_info)
				client_callback_infos.push_back(cal_callback_info);

			callback_info = get_callback_info(sensor_id, event_info, sensor_data);

			if (!callback_info) {
				_E("Failed to get callback_info");
				continue;
			}

			if (sensor_handle_info.m_accuracy != accuracy) {
				m_client_info.set_accuracy(sensor_handle_info.m_handle, accuracy);

				callback_info->accuracy_cb = sensor_handle_info.m_accuracy_cb;
				callback_info->timestamp = cur_time;
				callback_info->accuracy = accuracy;
				callback_info->accuracy_user_data = sensor_handle_info.m_accuracy_user_data;
			}

			client_callback_infos.push_back(callback_info);

			print_event_occurrence_log(sensor_handle_info);
		}
	}

	auto it_calback_info = client_callback_infos.begin();

	while (it_calback_info != client_callback_infos.end()) {
		post_callback_to_main_loop(*it_calback_info);
		++it_calback_info;
	}
}

client_callback_info* sensor_event_listener::get_callback_info(sensor_id_t sensor_id, const reg_event_info *event_info, std::shared_ptr<void> sensor_data)
{
	client_callback_info* callback_info;

	callback_info = new(std::nothrow)client_callback_info;
	retvm_if(!callback_info, NULL, "Failed to allocate memory");

	callback_info->sensor = sensor_info_to_sensor(sensor_info_manager::get_instance().get_info(sensor_id));
	callback_info->event_id = event_info->m_id;
	callback_info->handle = event_info->m_handle;
	callback_info->cb = event_info->m_cb;
	callback_info->event_type = event_info->type;
	callback_info->user_data = event_info->m_user_data;
	callback_info->accuracy_cb = NULL;
	callback_info->timestamp = 0;
	callback_info->accuracy = -1;
	callback_info->accuracy_user_data = NULL;
	callback_info->sensor_data = sensor_data;

	return callback_info;
}

void sensor_event_listener::post_callback_to_main_loop(client_callback_info* cb_info)
{
	g_idle_add_full(G_PRIORITY_DEFAULT, callback_dispatcher, cb_info, NULL);
}

bool sensor_event_listener::is_valid_callback(client_callback_info *cb_info)
{
	return m_client_info.is_event_active(cb_info->handle, cb_info->event_type, cb_info->event_id);
}

void sensor_event_listener::set_sensor_axis(int axis)
{
	m_axis = axis;
}

void sensor_event_listener::align_sensor_axis(sensor_t sensor, sensor_data_t *data)
{
	sensor_type_t type = sensor_to_sensor_info(sensor)->get_type();

	if (m_axis != SENSORD_AXIS_DISPLAY_ORIENTED)
		return;

	if (type != ACCELEROMETER_SENSOR && type != GYROSCOPE_SENSOR && type != GRAVITY_SENSOR && type != LINEAR_ACCEL_SENSOR)
		return;

	float x, y;

	switch (m_display_rotation) {
	case AUTO_ROTATION_DEGREE_90:	/* Landscape Left */
		x = -data->values[1];
		y = data->values[0];
		break;
	case AUTO_ROTATION_DEGREE_180:	/* Portrait Bottom */
		x = -data->values[0];
		y = -data->values[1];
		break;
	case AUTO_ROTATION_DEGREE_270:	/* Landscape Right */
		x = data->values[1];
		y = -data->values[0];
		break;
	default:
		return;
	}

	data->values[0] = x;
	data->values[1] = y;
}

gboolean sensor_event_listener::callback_dispatcher(gpointer data)
{
	client_callback_info *cb_info = (client_callback_info*) data;

	if (!sensor_event_listener::get_instance().is_valid_callback(cb_info)) {
		_W("Discard invalid callback cb(%#x)(%s, %#x, %#x) with id: %llu",
		cb_info->cb, get_event_name(cb_info->event_type), cb_info->sensor_data.get(),
		cb_info->user_data, cb_info->event_id);

		delete cb_info;
		return false;
	}

	if (cb_info->accuracy_cb)
		cb_info->accuracy_cb(cb_info->sensor, cb_info->timestamp, cb_info->accuracy, cb_info->accuracy_user_data);

	sensor_event_listener::get_instance().align_sensor_axis(cb_info->sensor, (sensor_data_t *)cb_info->sensor_data.get());
	((sensor_cb_t) cb_info->cb)(cb_info->sensor, cb_info->event_type, (sensor_data_t *)cb_info->sensor_data.get(), cb_info->user_data);

	delete cb_info;

/*
* 	To be called only once, it returns false
*/
	return false;
}

ssize_t sensor_event_listener::sensor_event_poll(void* buffer, int buffer_len, struct epoll_event &event)
{
	ssize_t len;

	len = m_event_socket.recv(buffer, buffer_len);

	if (!len) {
		if (!m_poller->poll(event))
			return -1;
		len = m_event_socket.recv(buffer, buffer_len);

		if (!len) {
			_I("%s failed to read after poll!", get_client_name());
			return -1;
		}
	}

	if (len < 0) {
		_I("%s failed to recv event from event socket", get_client_name());
		return -1;
	}

	return len;
}

void sensor_event_listener::listen_events(void)
{
	char buffer[EVENT_BUFFER_SIZE];
	struct epoll_event event;
	ssize_t len = -1;

	event.events = EPOLLIN | EPOLLPRI;

	do {
		void *buffer_data;
		int data_len;

		lock l(m_thread_mutex);
		if (m_thread_state != THREAD_STATE_START)
			break;

		len = sensor_event_poll(buffer, sizeof(sensor_event_t), event);
		if (len <= 0) {
			_E("Failed to sensor_event_poll()");
			break;
		}

		sensor_event_t *sensor_event = reinterpret_cast<sensor_event_t *>(buffer);
		data_len = sensor_event->data_length;
		buffer_data = malloc(data_len);

		len = sensor_event_poll(buffer_data, data_len, event);
		if (len <= 0) {
			_E("Failed to sensor_event_poll() for sensor_data");
			free(buffer_data);
			break;
		}

		sensor_event->data = reinterpret_cast<sensor_data_t *>(buffer_data);

		handle_events((void *)buffer);
	} while (true);

	if (m_poller != NULL) {
		delete m_poller;
		m_poller = NULL;
	}

	close_event_channel();

	{ /* the scope for the lock */
		lock l(m_thread_mutex);
		m_thread_state = THREAD_STATE_TERMINATE;
		m_thread_cond.notify_one();
	}

	_I("Event listener thread is terminated.");

	if (m_client_info.has_client_id() && (event.events & EPOLLHUP)) {
		if (m_hup_observer)
			m_hup_observer();
	}
}

bool sensor_event_listener::create_event_channel(void)
{
	const int client_type = CLIENT_TYPE_SENSOR_CLIENT;
	int client_id;
	channel_ready_t event_channel_ready;

	if (!m_event_socket.create(SOCK_SEQPACKET))
		return false;

	if (!m_event_socket.connect(EVENT_CHANNEL_PATH)) {
		_E("Failed to connect event channel for client %s, event socket fd[%d]", get_client_name(), m_event_socket.get_socket_fd());
		return false;
	}

	if (!m_event_socket.set_connection_mode()) {
		_E("Failed to set connection mode for client %s", get_client_name());
		return false;
	}

	if (m_event_socket.send(&client_type, sizeof(client_type)) <= 0) {
		_E("Failed to send client type in client %s, event socket fd[%d]", get_client_name(), m_event_socket.get_socket_fd());
		return false;
	}

	client_id = m_client_info.get_client_id();

	if (m_event_socket.send(&client_id, sizeof(client_id)) <= 0) {
		_E("Failed to send client id for client %s on event socket[%d]", get_client_name(), m_event_socket.get_socket_fd());
		return false;
	}

	if (m_event_socket.recv(&event_channel_ready, sizeof(event_channel_ready)) <= 0) {
		_E("%s failed to recv event_channel_ready packet on event socket[%d] with client id [%d]",
			get_client_name(), m_event_socket.get_socket_fd(), client_id);
		return false;
	}

	if ((event_channel_ready.magic != CHANNEL_MAGIC_NUM) || (event_channel_ready.client_id != client_id)) {
		_E("Event_channel_ready packet is wrong, magic = %#x, client id = %d",
			event_channel_ready.magic, event_channel_ready.client_id);
		return false;
	}

	_I("Event channel is established for client %s on socket[%d] with client id : %d",
		get_client_name(), m_event_socket.get_socket_fd(), client_id);

	return true;
}

void sensor_event_listener::close_event_channel(void)
{
	m_event_socket.close();
}

void sensor_event_listener::stop_event_listener(void)
{
	const int THREAD_TERMINATING_TIMEOUT = 2;

	ulock u(m_thread_mutex);

	if (m_thread_state != THREAD_STATE_TERMINATE) {
		m_thread_state = THREAD_STATE_STOP;

		_D("%s is waiting listener thread[state: %d] to be terminated", get_client_name(), m_thread_state);
		if (m_thread_cond.wait_for(u, std::chrono::seconds(THREAD_TERMINATING_TIMEOUT))
			== std::cv_status::timeout)
			_E("Fail to stop listener thread after waiting %d seconds", THREAD_TERMINATING_TIMEOUT);
		else
			_D("Listener thread for %s is terminated", get_client_name());
	}
}

void sensor_event_listener::set_thread_state(thread_state state)
{
	lock l(m_thread_mutex);
	m_thread_state = state;
}

void sensor_event_listener::clear(void)
{
	close_event_channel();
	stop_event_listener();
	m_client_info.close_command_channel();
	m_client_info.clear();
	m_client_info.set_client_id(CLIENT_ID_INVALID);
}

void sensor_event_listener::set_hup_observer(hup_observer_t observer)
{
	m_hup_observer = observer;
}

bool sensor_event_listener::start_event_listener(void)
{
	if (!create_event_channel()) {
		_E("Event channel is not established for %s", get_client_name());
		return false;
	}

	m_event_socket.set_transfer_mode();

	m_poller = new(std::nothrow) poller(m_event_socket.get_socket_fd());
	retvm_if(!m_poller, false, "Failed to allocate memory");

	set_thread_state(THREAD_STATE_START);

	thread listener(&sensor_event_listener::listen_events, this);
	listener.detach();

	return true;
}

void sensor_event_listener::set_display_rotation(int rt)
{
	_D("New display rotation: %d", rt);

	if (rt < AUTO_ROTATION_DEGREE_0 || rt > AUTO_ROTATION_DEGREE_270)
		return;

	m_display_rotation = rt;
}
