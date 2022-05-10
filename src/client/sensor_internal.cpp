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
#include <sensor_types_private.h>
#include <sensor_utils.h>

#include <channel_handler.h>
#include <sensor_manager.h>
#include <sensor_listener.h>
#include <sensor_provider.h>
#include <sensor_log.h>
#include <unordered_map>
#include <regex>
#include <thread>
#include <cmutex.h>
#include <command_types.h>

#include "sensor_reader.h"

#define CONVERT_OPTION_TO_PAUSE_POLICY(option) ((option) ^ 0b11)
#define MAX_LISTENER 100
#define MAX_PROVIDER 20

using namespace sensor;

typedef struct {
	int listener_id;
	sensor_info *sensor;
	void* cb;
	char* data;
	size_t data_size;
	void *user_data;
} callback_info_s;

typedef GSourceFunc callback_dispatcher_t;

static sensor::sensor_manager manager;
static std::unordered_map<int, sensor::sensor_listener *> listeners;
static cmutex lock;
static uint providerCnt = 0;

static gboolean sensor_events_callback_dispatcher(gpointer data)
{
	int event_type = 0;
	callback_info_s *info = (callback_info_s *)data;

	AUTOLOCK(lock);

	if (info->sensor)
		event_type = CONVERT_TYPE_EVENT(info->sensor->get_type());

	if (info->cb && info->sensor && listeners.find(info->listener_id) != listeners.end()) {
		size_t element_size =  sizeof(sensor_data_t);
		size_t count = info->data_size / element_size;
		((sensor_events_cb_t)info->cb)(info->sensor, event_type, (sensor_data_t*)info->data, count, info->user_data);
	}

	delete [] info->data;
	delete info;
	return FALSE;
}

static gboolean sensor_event_callback_dispatcher(gpointer data)
{
	int event_type = 0;
	callback_info_s *info = (callback_info_s *)data;

	AUTOLOCK(lock);

	if (info->sensor)
		event_type = CONVERT_TYPE_EVENT(info->sensor->get_type());

	if (info->cb && info->sensor && listeners.find(info->listener_id) != listeners.end()) {
		((sensor_cb_t)info->cb)(info->sensor, event_type, (sensor_data_t*)info->data, info->user_data);
	}

	delete [] info->data;
	delete info;
	return FALSE;
}

static gboolean sensor_accuracy_changed_callback_dispatcher(gpointer data)
{
	callback_info_s *info = (callback_info_s *)data;

	AUTOLOCK(lock);

	if (info->cb && info->sensor && listeners.find(info->listener_id) != listeners.end()) {
		sensor_data_t * sensor_data = (sensor_data_t *)info->data;
		((sensor_accuracy_changed_cb_t)info->cb)(info->sensor, sensor_data->timestamp, sensor_data->accuracy, info->user_data);
	}

	delete [] info->data;
	delete info;
	return FALSE;
}

static gboolean sensor_attribute_int_changed_callback_dispatcher(gpointer data)
{
	callback_info_s *info = (callback_info_s *)data;

	AUTOLOCK(lock);

	if (info->cb && info->sensor && listeners.find(info->listener_id) != listeners.end()) {
		cmd_listener_attr_int_t *d = (cmd_listener_attr_int_t *)info->data;
		((sensor_attribute_int_changed_cb_t)info->cb)(info->sensor, d->attribute, d->value, info->user_data);
	}

	delete [] info->data;
	delete info;
	return FALSE;
}

static gboolean sensor_attribute_str_changed_callback_dispatcher(gpointer data)
{
	callback_info_s *info = (callback_info_s *)data;

	AUTOLOCK(lock);

	if (info->cb && info->sensor && listeners.find(info->listener_id) != listeners.end()) {
		cmd_listener_attr_str_t *d = (cmd_listener_attr_str_t *)info->data;
		((sensor_attribute_str_changed_cb_t)info->cb)(info->sensor, d->attribute, d->value, d->len, info->user_data);
	}

	delete [] info->data;
	delete info;
	return FALSE;
}

class sensor_listener_channel_handler : public ipc::channel_handler
{
public:
	sensor_listener_channel_handler(int id, sensor_t sensor, void* cb, void *user_data, callback_dispatcher_t dispatcher)
	: m_listener_id(id)
	, m_sensor(reinterpret_cast<sensor_info *>(sensor))
	, m_cb(cb)
	, m_user_data(user_data)
	, m_dispatcher(dispatcher)
	{}

	void connected(ipc::channel *ch) {}
	void disconnected(ipc::channel *ch) {}
	void read(ipc::channel *ch, ipc::message &msg)
	{
		callback_info_s *info;
		auto size = msg.size();
		char *data = new(std::nothrow) char[size];
		if (data == NULL)
			return;
		memcpy(data, msg.body(), size);

		info = new(std::nothrow) callback_info_s();
		info->listener_id = m_listener_id;
		info->cb = m_cb;
		info->sensor = m_sensor;
		info->data = data;
		info->data_size = size;
		info->user_data = m_user_data;

		g_idle_add(m_dispatcher, info);
	}

	void read_complete(ipc::channel *ch) {}
	void error_caught(ipc::channel *ch, int error) {}
	void set_handler(int num, ipc::channel_handler* handler) {}
	void disconnect(void) {}

private:
	int m_listener_id;
	sensor_info *m_sensor;
	void* m_cb;
	void *m_user_data;
	callback_dispatcher_t m_dispatcher;
};

/*
 * TO-DO-LIST:
 * 1. power save option / lcd vconf : move to server
 * 2. thread-safe : ipc_client
 */

API int sensord_get_sensors(sensor_type_t type, sensor_t **list, int *count)
{
	return sensord_get_sensors_by_uri(utils::get_uri(type), list, count);
}

API int sensord_get_default_sensor(sensor_type_t type, sensor_t *sensor)
{
	return sensord_get_default_sensor_by_uri(utils::get_uri(type), sensor);
}

API bool sensord_get_type(sensor_t sensor, sensor_type_t *type)
{
	retvm_if(!type, false, "Invalid type");
	retvm_if(!manager.connect(), false, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), false,
			"Invalid sensor[%p]", sensor);

	*type = static_cast<sensor_info *>(sensor)->get_type();

	return true;
}

API const char* sensord_get_uri(sensor_t sensor)
{
	retvm_if(!manager.connect(), NULL, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), NULL,
			"Invalid sensor[%p]", sensor);

	return static_cast<sensor_info *>(sensor)->get_uri().c_str();
}

API const char* sensord_get_name(sensor_t sensor)
{
	retvm_if(!manager.connect(), NULL, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), NULL,
			"Invalid sensor[%p]", sensor);

	return static_cast<sensor_info *>(sensor)->get_model().c_str();
}

API const char* sensord_get_vendor(sensor_t sensor)
{
	retvm_if(!manager.connect(), NULL, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), NULL,
			"Invalid sensor[%p]", sensor);

	return static_cast<sensor_info *>(sensor)->get_vendor().c_str();
}

API bool sensord_get_min_range(sensor_t sensor, float *min_range)
{
	retvm_if(!min_range, false, "Invalid paramter");
	retvm_if(!manager.connect(), false, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), false,
			"Invalid sensor[%p]", sensor);

	*min_range = static_cast<sensor_info *>(sensor)->get_min_range();

	return true;
}

API bool sensord_get_max_range(sensor_t sensor, float *max_range)
{
	retvm_if(!max_range, false, "Invalid parameter");
	retvm_if(!manager.connect(), false, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), false,
			"Invalid sensor[%p]", sensor);

	*max_range = static_cast<sensor_info *>(sensor)->get_max_range();

	return true;
}

API bool sensord_get_resolution(sensor_t sensor, float *resolution)
{
	retvm_if(!resolution, false, "Invalid parameter");
	retvm_if(!manager.connect(), false, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), false,
			"Invalid sensor[%p]", sensor);

	*resolution = static_cast<sensor_info *>(sensor)->get_resolution();

	return true;
}

API bool sensord_get_min_interval(sensor_t sensor, int *min_interval)
{
	retvm_if(!min_interval, false, "Invalid parameter");
	retvm_if(!manager.connect(), false, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), false,
			"Invalid sensor[%p]", sensor);

	*min_interval = static_cast<sensor_info *>(sensor)->get_min_interval();

	return true;
}

API bool sensord_get_fifo_count(sensor_t sensor, int *fifo_count)
{
	retvm_if(!fifo_count, false, "Invalid parameter");
	retvm_if(!manager.connect(), false, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), false,
			"Invalid sensor[%p]", sensor);

	*fifo_count = 0;

	return true;
}

API bool sensord_get_max_batch_count(sensor_t sensor, int *max_batch_count)
{
	retvm_if(!max_batch_count, false, "Invalid parameter");
	retvm_if(!manager.connect(), false, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), false,
			"Invalid sensor[%p]", sensor);

	*max_batch_count = static_cast<sensor_info *>(sensor)->get_max_batch_count();

	return true;
}

API bool sensord_is_wakeup_supported(sensor_t sensor)
{
	retvm_if(!manager.connect(), false, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), false,
			"Invalid sensor[%p]", sensor);

	return static_cast<sensor_info *>(sensor)->is_wakeup_supported();
}

API int sensord_connect(sensor_t sensor)
{
	AUTOLOCK(lock);

	retvm_if(!manager.connect(), -EIO, "Failed to connect");
	retvm_if(!manager.is_supported(sensor), -EINVAL,
			"Invalid sensor[%p]", sensor);
	retvm_if(listeners.size() > MAX_LISTENER, -EPERM, "Exceeded the maximum listener");

	sensor::sensor_listener *listener;
	static sensor_reader reader;

	listener = new(std::nothrow) sensor::sensor_listener(sensor, reader.get_event_loop());
	retvm_if(!listener, -ENOMEM, "Failed to allocate memory");

	listeners[listener->get_id()] = listener;

	_D("Connect[%d]", listener->get_id());

	return listener->get_id();
}

API bool sensord_disconnect(int handle)
{
	sensor::sensor_listener *listener;

	AUTOLOCK(lock);

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;
	retvm_if(!listener, false, "Invalid handle[%d]", handle);

	_D("Disconnect[%d]", listener->get_id());

	delete listener;
	listeners.erase(handle);

	if (listeners.empty())
		manager.disconnect();

	return true;
}

static inline bool sensord_register_event_impl(int handle, unsigned int event_type,
		unsigned int interval, unsigned int max_batch_latency, void* cb, bool is_events_callback, void *user_data)
{
	sensor::sensor_listener *listener;
	int prev_interval;
	int prev_max_batch_latency;
	sensor_listener_channel_handler *handler;

	AUTOLOCK(lock);

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

	if (is_events_callback) {
		handler = new(std::nothrow)sensor_listener_channel_handler(handle, listener->get_sensor(), (void *)cb, user_data, sensor_events_callback_dispatcher);
	} else {
		handler = new(std::nothrow)sensor_listener_channel_handler(handle, listener->get_sensor(), (void *)cb, user_data, sensor_event_callback_dispatcher);
	}

	if (!handler) {
		listener->set_max_batch_latency(prev_max_batch_latency);
		listener->set_interval(prev_interval);
		_E("Failed to allocate memory");
		return false;
	}

	listener->set_event_handler(handler);

	_D("Register event[%d]", listener->get_id());

	return true;
}

API bool sensord_register_event(int handle, unsigned int event_type,
		unsigned int interval, unsigned int max_batch_latency, sensor_cb_t cb, void *user_data)
{
	return sensord_register_event_impl(handle, event_type, interval, max_batch_latency, (void*)cb, false, user_data);
}

static inline bool sensord_unregister_event_imple(int handle)
{
	sensor::sensor_listener *listener;

	AUTOLOCK(lock);

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	listener->unset_event_handler();

	_D("Unregister event[%d]", listener->get_id());

	return true;
}

API bool sensord_unregister_event(int handle, unsigned int event_type)
{
	return sensord_unregister_event_imple(handle);
}

API bool sensord_register_events(int handle, unsigned int event_type, unsigned int max_batch_latency, sensor_events_cb_t cb, void *user_data)
{
	return sensord_register_event_impl(handle, event_type, 0, max_batch_latency, (void*)cb, true, user_data);
}

API bool sensord_unregister_events(int handle, unsigned int event_type)
{
	return sensord_unregister_event_imple(handle);
}

API bool sensord_register_accuracy_cb(int handle, sensor_accuracy_changed_cb_t cb, void *user_data)
{
	sensor::sensor_listener *listener;
	sensor_listener_channel_handler *handler;

	AUTOLOCK(lock);

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	handler = new(std::nothrow) sensor_listener_channel_handler(handle, listener->get_sensor(), (void *)cb, user_data, sensor_accuracy_changed_callback_dispatcher);
	retvm_if(!handler, false, "Failed to allocate memory");

	listener->set_accuracy_handler(handler);

	return true;
}

API bool sensord_unregister_accuracy_cb(int handle)
{
	sensor::sensor_listener *listener;

	AUTOLOCK(lock);

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	listener->unset_accuracy_handler();

	return true;
}

API bool sensord_register_attribute_int_changed_cb(int handle, sensor_attribute_int_changed_cb_t cb, void *user_data)
{
	sensor::sensor_listener *listener;
	sensor_listener_channel_handler *handler;

	AUTOLOCK(lock);

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	handler = new(std::nothrow) sensor_listener_channel_handler(handle, listener->get_sensor(), (void *)cb, user_data, sensor_attribute_int_changed_callback_dispatcher);
	retvm_if(!handler, false, "Failed to allocate memory");

	listener->set_attribute_int_changed_handler(handler);

	return true;
}

API bool sensord_unregister_attribute_int_changed_cb(int handle)
{
	sensor::sensor_listener *listener;

	AUTOLOCK(lock);

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	listener->unset_attribute_int_changed_handler();

	return true;
}

API bool sensord_register_attribute_str_changed_cb(int handle, sensor_attribute_str_changed_cb_t cb, void *user_data)
{
	sensor::sensor_listener *listener;
	sensor_listener_channel_handler *handler;

	AUTOLOCK(lock);

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	handler = new(std::nothrow) sensor_listener_channel_handler(handle, listener->get_sensor(), (void *)cb, user_data, sensor_attribute_str_changed_callback_dispatcher);
	retvm_if(!handler, false, "Failed to allocate memory");

	listener->set_attribute_str_changed_handler(handler);

	return true;
}

API bool sensord_unregister_attribute_str_changed_cb(int handle)
{
	sensor::sensor_listener *listener;

	AUTOLOCK(lock);

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	listener->unset_attribute_str_changed_handler();

	return true;
}

API bool sensord_start(int handle, int option)
{
	sensor::sensor_listener *listener;
	int prev_pause;
	int pause;
	int interval, batch_latency;

	AUTOLOCK(lock);

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	pause = CONVERT_OPTION_TO_PAUSE_POLICY(option);
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

	interval = listener->get_interval();
	if (interval > 0)
		listener->set_interval(interval);

	batch_latency = listener->get_max_batch_latency();
	if (batch_latency != SENSOR_BATCH_LATENCY_DEFAULT)
		listener->set_max_batch_latency(batch_latency);

	_D("Start[%d] with the interval[%d] batch_latency[%d]",
		listener->get_id(), interval, batch_latency);

	return true;
}

API bool sensord_stop(int handle)
{
	int ret;
	sensor::sensor_listener *listener;

	AUTOLOCK(lock);

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	ret = listener->stop();

	if (ret == -EAGAIN || ret == OP_SUCCESS)
		return true;

	_D("Stop[%d]", listener->get_id());

	return false;
}

API bool sensord_change_event_interval(int handle, unsigned int event_type, unsigned int interval)
{
	sensor::sensor_listener *listener;

	AUTOLOCK(lock);

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	if (listener->set_interval(interval) < 0) {
		_E("Failed to set interval to listener");
		return false;
	}

	_D("Set interval[%d, %d]", listener->get_id(), interval);

	return true;
}

API bool sensord_change_event_max_batch_latency(int handle, unsigned int event_type, unsigned int max_batch_latency)
{
	sensor::sensor_listener *listener;

	AUTOLOCK(lock);

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	if (listener->set_max_batch_latency(max_batch_latency) < 0) {
		_E("Failed to set max_batch_latency to listener");
		return false;
	}

	_D("Set max batch latency[%d, %u]", listener->get_id(), max_batch_latency);

	return true;
}

API bool sensord_set_option(int handle, int option)
{
	sensor::sensor_listener *listener;
	int pause;

	AUTOLOCK(lock);

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	pause = CONVERT_OPTION_TO_PAUSE_POLICY(option);

	if (listener->set_attribute(SENSORD_ATTRIBUTE_PAUSE_POLICY, pause) < 0) {
		_E("Failed to set option[%d(%d)] to listener", option, pause);
		return false;
	}

	_D("Set pause option[%d, %d]", listener->get_id(), pause);

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

	_D("Set attribute[%d, %d, %d]", listener->get_id(), attribute, value);

	return OP_SUCCESS;
}

API int sensord_get_attribute_int(int handle, int attribute, int* value)
{
	sensor::sensor_listener *listener;

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), -EINVAL, "Invalid handle[%d]", handle);

	listener = it->second;

	if (listener->get_attribute(attribute, value) < 0) {
		_E("Failed to get attribute[%d]", attribute);
		return -EIO;
	}

	_D("Get attribute[%d, %d, %d]", listener->get_id(), attribute, *value);

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
	_D("Set attribute ID[%d], attr[%d], len[%d]", listener->get_id(), attribute, len);

	return OP_SUCCESS;
}

API int sensord_get_attribute_str(int handle, int attribute, char **value, int* len)
{
	sensor::sensor_listener *listener;

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), -EINVAL, "Invalid handle[%d]", handle);

	listener = it->second;

	if (listener->get_attribute(attribute, value, len) < 0) {
		_E("Failed to get attribute[%d]", attribute);
		return -EIO;
	}
	_D("Get attribute ID[%d], attr[%d], len[%d]", listener->get_id(), attribute, *len);

	return OP_SUCCESS;
}

API bool sensord_get_data(int handle, unsigned int data_id, sensor_data_t* sensor_data)
{
	sensor::sensor_listener *listener;

	AUTOLOCK(lock);

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	if (listener->get_sensor_data(sensor_data) < 0) {
		_E("Failed to get sensor data from listener");
		return false;
	}

	return true;
}

API bool sensord_get_data_list(int handle, unsigned int data_id, sensor_data_t** sensor_data, int* count)
{
	sensor::sensor_listener *listener;

	AUTOLOCK(lock);

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	if (listener->get_sensor_data_list(sensor_data, count) < 0) {
		_E("Failed to get sensor data from listener");
		return false;
	}

	return true;
}

API bool sensord_flush(int handle)
{
	sensor::sensor_listener *listener;

	AUTOLOCK(lock);

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

	AUTOLOCK(lock);

	auto it = listeners.find(handle);
	retvm_if(it == listeners.end(), false, "Invalid handle[%d]", handle);

	listener = it->second;

	if (listener->set_passive_mode(passive) < 0) {
		_E("Failed to set passive mode");
		return false;
	}

	return true;
}

/* Sensor Internal API using URI */
API int sensord_get_default_sensor_by_uri(const char *uri, sensor_t *sensor)
{
	retvm_if(!sensor, -EINVAL, "Invalid parameter");
	retvm_if(!manager.connect(), -EIO, "Failed to connect");

	return manager.get_sensor(uri, sensor);
}

API int sensord_get_sensors_by_uri(const char *uri, sensor_t **list, int *count)
{
	retvm_if((!list || !count), -EINVAL, "Invalid parameter");
	retvm_if(!manager.connect(), -EIO, "Failed to connect");

	return manager.get_sensors(uri, list, count);
}

API int sensord_add_sensor_added_cb(sensord_added_cb callback, void *user_data)
{
	retvm_if(!callback, -EINVAL, "Invalid paramter");
	retvm_if(!manager.connect(), -EIO, "Failed to connect");

	manager.add_sensor_added_cb(callback, user_data);
	return OP_SUCCESS;
}

API int sensord_remove_sensor_added_cb(sensord_added_cb callback)
{
	retvm_if(!callback, -EINVAL, "Invalid paramter");
	retvm_if(!manager.connect(), -EIO, "Failed to connect");

	manager.remove_sensor_added_cb(callback);
	return OP_SUCCESS;
}

API int sensord_add_sensor_removed_cb(sensord_removed_cb callback, void *user_data)
{
	retvm_if(!callback, -EINVAL, "Invalid paramter");
	retvm_if(!manager.connect(), -EIO, "Failed to connect");

	manager.add_sensor_removed_cb(callback, user_data);
	return OP_SUCCESS;
}

API int sensord_remove_sensor_removed_cb(sensord_removed_cb callback)
{
	retvm_if(!callback, -EINVAL, "Invalid paramter");
	retvm_if(!manager.connect(), -EIO, "Failed to connect");

	manager.remove_sensor_removed_cb(callback);
	return OP_SUCCESS;
}

/* Sensor provider */
API int sensord_create_provider(const char *uri, sensord_provider_h *provider)
{
	retvm_if(providerCnt >= MAX_PROVIDER, -EPERM, "Exceeded the maximum provider");
	retvm_if(!provider, -EINVAL, "Invalid paramter");

	std::string str_uri(uri);
	retvm_if(str_uri.find(PREDEFINED_TYPE_URI) != std::string::npos,
			-EINVAL, "Invalid URI format[%s]", uri);

	static std::regex uri_regex(SENSOR_URI_REGEX, std::regex::optimize);
	retvm_if(!std::regex_match(uri, uri_regex),
			-EINVAL, "Invalid URI format[%s]", uri);

	sensor_provider *p;

	p = new(std::nothrow) sensor_provider(uri);
	retvm_if(!p, -ENOMEM, "Failed to allocate memory");

	*provider = static_cast<sensord_provider_h>(p);
	providerCnt++;
	return OP_SUCCESS;
}

API int sensord_destroy_provider(sensord_provider_h provider)
{
	retvm_if(!provider, -EINVAL, "Invalid paramter");

	delete static_cast<sensor::sensor_provider *>(provider);
	providerCnt--;
	return OP_SUCCESS;
}

API int sensord_add_provider(sensord_provider_h provider)
{
	retvm_if(!provider, -EINVAL, "Invalid paramter");
	retvm_if(!manager.connect(), -EIO, "Failed to connect");

	int ret;
	sensor_provider *p = static_cast<sensor_provider *>(provider);

	ret = p->connect();
	retv_if(ret < 0, ret);

	ret = manager.add_sensor(p);
	if (ret < 0) {
		p->disconnect();
		return ret;
	}

	return OP_SUCCESS;
}

API int sensord_remove_provider(sensord_provider_h provider)
{
	retvm_if(!provider, -EINVAL, "Invalid paramter");
	retvm_if(!manager.connect(), -EIO, "Failed to connect");

	int ret;
	sensor_provider *p = static_cast<sensor_provider *>(provider);

	if (!p->disconnect())
		return OP_ERROR;

	ret = manager.remove_sensor(p);
	if (ret < 0) {
		p->connect();
		return OP_ERROR;
	}

	return OP_SUCCESS;
}

API int sensord_provider_set_name(sensord_provider_h provider, const char *name)
{
	retvm_if(!provider, -EINVAL, "Invalid paramter");

	sensor_provider *p = static_cast<sensor_provider *>(provider);

	sensor_info *info = p->get_sensor_info();
	info->set_model(name);

	return OP_SUCCESS;
}

API int sensord_provider_set_vendor(sensord_provider_h provider, const char *vendor)
{
	retvm_if(!provider, -EINVAL, "Invalid paramter");

	sensor_provider *p = static_cast<sensor_provider *>(provider);

	sensor_info *info = p->get_sensor_info();
	info->set_vendor(vendor);

	return OP_SUCCESS;
}

API int sensord_provider_set_range(sensord_provider_h provider, float min_range, float max_range)
{
	retvm_if(!provider, -EINVAL, "Invalid paramter");

	sensor_provider *p = static_cast<sensor_provider *>(provider);

	sensor_info *info = p->get_sensor_info();
	info->set_min_range(min_range);
	info->set_max_range(max_range);

	return OP_SUCCESS;
}

API int sensord_provider_set_resolution(sensord_provider_h provider, float resolution)
{
	retvm_if(!provider, -EINVAL, "Invalid paramter");

	sensor_provider *p = static_cast<sensor_provider *>(provider);

	sensor_info *info = p->get_sensor_info();
	info->set_resolution(resolution);

	return OP_SUCCESS;
}

API int sensord_provider_set_start_cb(sensord_provider_h provider, sensord_provider_start_cb callback, void *user_data)
{
	retvm_if(!provider, -EINVAL, "Invalid paramter");
	retvm_if(!callback, -EINVAL, "Invalid paramter");

	sensor_provider *p = static_cast<sensor_provider *>(provider);

	p->set_start_cb(callback, user_data);

	return OP_SUCCESS;
}

API int sensord_provider_set_stop_cb(sensord_provider_h provider, sensord_provider_stop_cb callback, void *user_data)
{
	retvm_if(!provider, -EINVAL, "Invalid paramter");
	retvm_if(!callback, -EINVAL, "Invalid paramter");

	sensor_provider *p = static_cast<sensor_provider *>(provider);

	p->set_stop_cb(callback, user_data);

	return OP_SUCCESS;
}

API int sensord_provider_set_interval_changed_cb(sensord_provider_h provider, sensord_provider_interval_changed_cb callback, void *user_data)
{
	retvm_if(!provider, -EINVAL, "Invalid paramter");
	retvm_if(!callback, -EINVAL, "Invalid paramter");

	sensor_provider *p = static_cast<sensor_provider *>(provider);

	p->set_interval_cb(callback, user_data);

	return OP_SUCCESS;
}

API int sensord_provider_set_attribute_str_cb(sensord_provider_h provider, sensord_provider_attribute_str_cb callback, void *user_data)
{
	retvm_if(!provider, -EINVAL, "Invalid paramter");
	retvm_if(!callback, -EINVAL, "Invalid paramter");

	sensor_provider *p = static_cast<sensor_provider *>(provider);

	p->set_attribute_str_cb(callback, user_data);

	return OP_SUCCESS;
}

API int sensord_provider_publish(sensord_provider_h provider, sensor_data_t data)
{
	retvm_if(!provider, -EINVAL, "Invalid paramter");

	sensor_provider *p = static_cast<sensor_provider *>(provider);

	/* TODO: synchronous call is enough? */
	return p->publish(data);
}

API int sensord_provider_publish_events(sensord_provider_h provider, sensor_data_t events[], int count)
{
	retvm_if(!provider, -EINVAL, "Invalid paramter");

	sensor_provider *p = static_cast<sensor_provider *>(provider);

	return p->publish(events, count);
};

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

/* deprecated */
API int sensord_external_connect(const char *key, sensor_external_command_cb_t cb, void *user_data)
{
	/*
	 * 1. check parameter
	 * 2. create handle in this client
	 * 3. first connection(client)
	 * 4. cmd_connect for external sensor with key
	 */
	retvm_if(!key, -EINVAL, "Invalid key");
	return 0;
}

/* deprecated */
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

/* deprecated */
API bool sensord_external_post(int handle, unsigned long long timestamp, const float* data, int data_cnt)
{
	/*
	 * 1. check parameter
	 * 1.1 (data_cnt <= 0) || (data_cnt > POST_DATA_LEN_MAX)), return false
	 * 2. cmd_post
	 */

	return true;
}

