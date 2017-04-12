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

#include <sensor_internal.h>
#include <sensor_internal_deprecated.h>
#include <sensor_types.h>

#include <channel_handler.h>
#include <sensor_manager.h>
#include <sensor_listener.h>
#include <sensor_log.h>
#include <unordered_map>

#define CONVERT_OPTION_PAUSE_POLICY(option) ((option) ^ 0b11)

using namespace sensor;

class sensor_event_handler : public ipc::channel_handler
{
public:
	sensor_event_handler(sensor_t sensor, sensor_cb_t cb, void *user_data)
	: m_sensor(reinterpret_cast<sensor_info *>(sensor))
	, m_cb(cb)
	, m_user_data(user_data)
	{}

	void connected(ipc::channel *ch) {}
	void disconnected(ipc::channel *ch) {}
	void read(ipc::channel *ch, ipc::message &msg)
	{
		int event_type;
		sensor_data_t *data;

		data = reinterpret_cast<sensor_data_t *>(msg.body());
		event_type = CONVERT_TYPE_EVENT(m_sensor->get_type());

		m_cb(m_sensor, event_type, data, m_user_data);
	}

	void read_complete(ipc::channel *ch) {}
	void error_caught(ipc::channel *ch, int error) {}

private:
	sensor_info *m_sensor;
	sensor_cb_t m_cb;
	void *m_user_data;
};

class sensor_accuracy_handler : public ipc::channel_handler
{
public:
	sensor_accuracy_handler(sensor_t sensor, sensor_accuracy_changed_cb_t cb, void *user_data)
	: m_sensor(reinterpret_cast<sensor_info *>(sensor))
	, m_cb(cb)
	, m_user_data(user_data)
	{}

	void connected(ipc::channel *ch) {}
	void disconnected(ipc::channel *ch) {}
	void read(ipc::channel *ch, ipc::message &msg)
	{
		sensor_data_t *data;
		data = reinterpret_cast<sensor_data_t *>(msg.body());

		m_cb(m_sensor, data->timestamp, data->accuracy, m_user_data);
	}

	void read_complete(ipc::channel *ch) {}
	void error_caught(ipc::channel *ch, int error) {}

private:
	sensor_info *m_sensor;
	sensor_accuracy_changed_cb_t m_cb;
	void *m_user_data;
};

static sensor::sensor_manager manager;
static std::unordered_map<int, sensor::sensor_listener *> listeners;

/*
 * TO-DO-LIST:
 * 1. power save option / lcd vconf : move to server
 * 2. thread-safe : ipc_client
 */

API int sensord_get_sensors(sensor_type_t type, sensor_t **list, int *count)
{
	int ret = OP_ERROR;

	retvm_if((!list || !count), -EINVAL,
			"Invalid parameter[%#x, %#x]", list, count);
	retvm_if(!manager.connect(), -EIO, "Failed to connect");

	ret = manager.get_sensors(type, list, count);
	retv_if(ret < 0, ret);

	return OP_SUCCESS;
}

API int sensord_get_default_sensor(sensor_type_t type, sensor_t *sensor)
{
	int ret = OP_ERROR;

	retvm_if(!sensor, -EPERM, "Invalid parameter[%#x]", sensor);
	retvm_if(!manager.connect(), -EIO, "Failed to connect");

	ret = manager.get_sensor(type, sensor);
	retv_if(ret < 0, ret);

	return OP_SUCCESS;
}

API bool sensord_get_type(sensor_t sensor, sensor_type_t *type)
{
	retvm_if(!type, false, "Invalid parameter[%#x]", type);
	retvm_if(!manager.connect(), false, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), false,
			"Invalid sensor[%#x]", sensor);

	*type = static_cast<sensor_info *>(sensor)->get_type();

	return true;
}

API const char* sensord_get_name(sensor_t sensor)
{
	retvm_if(!manager.connect(), NULL, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), NULL,
			"Invalid sensor[%#x]", sensor);

	return static_cast<sensor_info *>(sensor)->get_uri().c_str();
}

API const char* sensord_get_vendor(sensor_t sensor)
{
	retvm_if(!manager.connect(), NULL, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), NULL,
			"Invalid sensor[%#x]", sensor);

	return static_cast<sensor_info *>(sensor)->get_vendor().c_str();
}

API bool sensord_get_min_range(sensor_t sensor, float *min_range)
{
	retvm_if(!manager.connect(), false, "Failed to connect");
	retvm_if(!min_range, false, "Invalid parameter[%#x]", min_range);
	retvm_if(!manager.is_supported(sensor), false,
			"Invalid sensor[%#x]", sensor);

	*min_range = static_cast<sensor_info *>(sensor)->get_min_range();

	return true;
}

API bool sensord_get_max_range(sensor_t sensor, float *max_range)
{
	retvm_if(!max_range, false, "Invalid parameter[%#x]", max_range);
	retvm_if(!manager.connect(), false, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), false,
			"Invalid sensor[%#x]", sensor);

	*max_range = static_cast<sensor_info *>(sensor)->get_max_range();

	return true;
}

API bool sensord_get_resolution(sensor_t sensor, float *resolution)
{
	retvm_if(!resolution, false, "Invalid parameter[%#x]", resolution);
	retvm_if(!manager.connect(), false, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), false,
			"Invalid sensor[%#x]", sensor);

	*resolution = static_cast<sensor_info *>(sensor)->get_resolution();

	return true;
}

API bool sensord_get_min_interval(sensor_t sensor, int *min_interval)
{
	retvm_if(!min_interval, false, "Invalid parameter[%#x]", min_interval);
	retvm_if(!manager.connect(), false, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), false,
			"Invalid sensor[%#x]", sensor);

	*min_interval = static_cast<sensor_info *>(sensor)->get_min_interval();

	return true;
}

API bool sensord_get_fifo_count(sensor_t sensor, int *fifo_count)
{
	retvm_if(!fifo_count, false, "Invalid parameter[%#x]", fifo_count);
	retvm_if(!manager.connect(), false, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), false,
			"Invalid sensor[%#x]", sensor);

	*fifo_count = 0;

	return true;
}

API bool sensord_get_max_batch_count(sensor_t sensor, int *max_batch_count)
{
	retvm_if(!max_batch_count, false, "Invalid parameter[%#x]", max_batch_count);
	retvm_if(!manager.connect(), false, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), false,
			"Invalid sensor[%#x]", sensor);

	*max_batch_count = static_cast<sensor_info *>(sensor)->get_max_batch_count();

	return true;
}

API bool sensord_is_wakeup_supported(sensor_t sensor)
{
	retvm_if(!manager.connect(), false, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), false,
			"Invalid sensor[%#x]", sensor);

	return static_cast<sensor_info *>(sensor)->is_wakeup_supported();
}

API int sensord_connect(sensor_t sensor)
{
	retvm_if(!manager.connect(), -EIO, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), -EINVAL,
			"Invalid sensor[%#x]", sensor);

	sensor::sensor_listener *listener;

	listener = new(std::nothrow) sensor::sensor_listener(sensor);
	retvm_if(!listener, -ENOMEM, "Failed to allocate memory");

	listeners[listener->get_id()] = listener;

	return listener->get_id();
}

API bool sensord_disconnect(int handle)
{
	sensor::sensor_listener *listener;

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;
	retvm_if(!listener, false, "Invalid handle[%d]", handle);

	delete listener;
	listeners.erase(handle);

	return true;
}

API bool sensord_register_event(int handle, unsigned int event_type,
		unsigned int interval, unsigned int max_batch_latency, sensor_cb_t cb, void *user_data)
{
	sensor::sensor_listener *listener;
	int prev_interval;
	int prev_max_batch_latency;
	sensor_event_handler *handler;

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	prev_interval = listener->get_interval();
	prev_max_batch_latency = listener->get_max_batch_latency();

	if (listener->set_interval(interval) < 0) {
		_E("Failed to set interval");
		return false;
	}

	if (listener->set_max_batch_latency(max_batch_latency) < 0) {
		listener->set_interval(prev_interval);
		_E("Failed to set max_batch_latency");
		return false;
	}

	handler = new(std::nothrow) sensor_event_handler(listener->get_sensor(), cb, user_data);
	if (!handler) {
		listener->set_max_batch_latency(prev_max_batch_latency);
		listener->set_interval(prev_interval);
		_E("Failed to allocate memory");
		return false;
	}

	listener->set_event_handler(handler);

	return true;
}

API bool sensord_unregister_event(int handle, unsigned int event_type)
{
	sensor::sensor_listener *listener;

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	listener->unset_event_handler();

	return true;
}

API bool sensord_register_accuracy_cb(int handle, sensor_accuracy_changed_cb_t cb, void *user_data)
{
	sensor::sensor_listener *listener;
	sensor_accuracy_handler *handler;

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	handler = new(std::nothrow) sensor_accuracy_handler(listener->get_sensor(), cb, user_data);
	retvm_if(!handler, false, "Failed to allocate memory");

	listener->set_accuracy_handler(handler);

	return true;
}

API bool sensord_unregister_accuracy_cb(int handle)
{
	sensor::sensor_listener *listener;

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	listener->unset_accuracy_handler();

	return true;
}

API bool sensord_start(int handle, int option)
{
	sensor::sensor_listener *listener;
	int prev_pause;
	int pause;

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	pause = CONVERT_OPTION_PAUSE_POLICY(option);
	prev_pause = listener->get_pause_policy();

	if (listener->set_attribute(SENSORD_ATTRIBUTE_PAUSE_POLICY, pause) < 0) {
		_E("Failed to set pause policy[%d]", pause);
		return false;
	}

	if (listener->start() < 0) {
		listener->set_attribute(SENSORD_ATTRIBUTE_PAUSE_POLICY, prev_pause);
		_E("Failed to start listener");
		return false;
	}

	return true;
}

API bool sensord_stop(int handle)
{
	int ret;
	sensor::sensor_listener *listener;

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	ret = listener->stop();

	if (ret == -EAGAIN || ret == OP_SUCCESS)
		return true;

	return false;
}

API bool sensord_change_event_interval(int handle, unsigned int event_type, unsigned int interval)
{
	sensor::sensor_listener *listener;

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	if (listener->set_interval(interval) < 0) {
		_E("Failed to set interval to listener");
		return false;
	}

	return true;
}

API bool sensord_change_event_max_batch_latency(int handle, unsigned int event_type, unsigned int max_batch_latency)
{
	sensor::sensor_listener *listener;

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	if (listener->set_max_batch_latency(max_batch_latency) < 0) {
		_E("Failed to set max_batch_latency to listener");
		return false;
	}

	return true;
}

API bool sensord_set_option(int handle, int option)
{
	sensor::sensor_listener *listener;
	int pause;

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	pause = CONVERT_OPTION_PAUSE_POLICY(option);

	if (listener->set_attribute(SENSORD_ATTRIBUTE_PAUSE_POLICY, pause) < 0) {
		_E("Failed to set option[%d(%d)] to listener", option, pause);
		return false;
	}

	return true;
}

API int sensord_set_attribute_int(int handle, int attribute, int value)
{
	sensor::sensor_listener *listener;

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), -EINVAL, "Invalid handle[%d]", handle);

	listener = it->second;

	if (listener->set_attribute(attribute, value) < 0) {
		_E("Failed to set attribute[%d, %d]", attribute, value);
		return -EIO;
	}

	return OP_SUCCESS;
}

API int sensord_set_attribute_str(int handle, int attribute, const char *value, int len)
{
	sensor::sensor_listener *listener;

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), -EINVAL, "Invalid handle[%d]", handle);

	listener = it->second;

	if (listener->set_attribute(attribute, value, len) < 0) {
		_E("Failed to set attribute[%d, %s]", attribute, value);
		return -EIO;
	}

	return OP_SUCCESS;
}

API bool sensord_get_data(int handle, unsigned int data_id, sensor_data_t* sensor_data)
{
	sensor::sensor_listener *listener;

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	if (listener->get_sensor_data(sensor_data) < 0) {
		_E("Failed to get sensor data from listener");
		return false;
	}

	return true;
}

API bool sensord_flush(int handle)
{
	sensor::sensor_listener *listener;

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	if (listener->flush() < 0) {
		_E("Failed to flush sensor");
		return false;
	}

	return true;
}

API bool sensord_set_passive_mode(int handle, bool passive)
{
	sensor::sensor_listener *listener;

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	if (listener->set_passive_mode(passive) < 0) {
		_E("Failed to set passive mode");
		return false;
	}

	return true;
}

API int sensord_external_connect(const char *key, sensor_external_command_cb_t cb, void *user_data)
{
	/*
	 * 1. check parameter
	 * 2. create handle in this client
	 * 3. first connection(client)
	 * 4. cmd_connect for external sensor with key
	 */
	retvm_if(!key, -EPERM, "Invalid key");
	return 0;
}

API bool sensord_external_disconnect(int handle)
{
	/*
	 * 1. check parameter
	 * 2. create handle in this client
	 * 3. first connection(client)
	 * 4. cmd_connect for external sensor with key
	 * 5. disconnect this handle
	 * 6. if there is no active sensor, remove client id and stop listener
	 */
	return true;
}

API bool sensord_external_post(int handle, unsigned long long timestamp, const float* data, int data_cnt)
{
	/*
	 * 1. check parameter
	 * 1.1 (data_cnt <= 0) || (data_cnt > POST_DATA_LEN_MAX)), return false
	 * 2. cmd_post
	 */

	return true;
}

/* deperecated */
API sensor_t sensord_get_sensor(sensor_type_t type)
{
	sensor_t sensor;

	if (sensord_get_default_sensor(type, &sensor) < 0)
		return NULL;

	return sensor;
}

/* deprecated */
API bool sensord_get_sensor_list(sensor_type_t type, sensor_t **list, int *sensor_count)
{
	return (sensord_get_sensors(type, list, sensor_count) == OP_SUCCESS);
}

/* deprecated */
API bool sensord_register_hub_event(int handle, unsigned int event_type,
		unsigned int interval, unsigned int max_batch_latency, sensorhub_cb_t cb, void *user_data)
{
	return false;
}

/* deprecated */
API bool sensord_get_supported_event_types(sensor_t sensor, unsigned int **event_types, int *count)
{
	/*
	 * 1. check parameter
	 * 2. if there is no sensor, return false
	 * 3. memory allocation
	 */
	return true;
}

/* deprecated(BUT it is used in C-API....) */
API bool sensord_is_supported_event_type(sensor_t sensor, unsigned int event_type, bool *supported)
{
	if (!manager.is_supported(sensor))
		*supported = false;
	else
		*supported = true;

	return true;
}

/* deprecated */
API bool sensord_send_sensorhub_data(int handle, const char *data, int data_len)
{
	return (sensord_set_attribute_str(handle, 0, data, data_len) == OP_SUCCESS);
}

/* deprecated */
API bool sensord_send_command(int handle, const char *command, int command_len)
{
	return (sensord_set_attribute_str(handle, 0, command, command_len) == OP_SUCCESS);
}

/* deprecated */
API bool sensord_get_privilege(sensor_t sensor, sensor_privilege_t *privilege)
{
	*privilege = SENSOR_PRIVILEGE_PUBLIC;

	return true;
}

