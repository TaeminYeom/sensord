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

#include "sensor_listener_proxy.h"

#include <channel.h>
#include <message.h>
#include <command_types.h>
#include <sensor_log.h>
#include <sensor_types.h>

#include "sensor_handler.h"
#include "sensor_policy_monitor.h"

using namespace sensor;

sensor_listener_proxy::sensor_listener_proxy(uint32_t id,
			std::string uri, sensor_manager *manager, ipc::channel *ch)
: m_id(id)
, m_uri(uri)
, m_manager(manager)
, m_ch(ch)
, m_started(false)
, m_passive(false)
, m_pause_policy(SENSORD_PAUSE_ALL)
, m_axis_orientation(SENSORD_AXIS_DISPLAY_ORIENTED)
, m_last_accuracy(SENSOR_ACCURACY_UNDEFINED)
, m_need_to_notify_attribute_changed(false)
{
	_D("Create [%p][%s]", this, m_uri.data());
	sensor_policy_monitor::get_instance().add_listener(this);
}

sensor_listener_proxy::~sensor_listener_proxy()
{
	_D("Delete [%p][%s]", this, m_uri.data());
	sensor_policy_monitor::get_instance().remove_listener(this);
	stop();
}

uint32_t sensor_listener_proxy::get_id(void)
{
	return m_id;
}

int sensor_listener_proxy::update(const char *uri, std::shared_ptr<ipc::message> msg)
{
	retv_if(!m_ch || !m_ch->is_connected(), OP_CONTINUE);

	update_event(msg);
	update_accuracy(msg);

	return OP_CONTINUE;
}

int sensor_listener_proxy::on_attribute_changed(std::shared_ptr<ipc::message> msg)
{
	retv_if(!m_ch || !m_ch->is_connected(), OP_CONTINUE);
	_I("Proxy[%zu] call on_attribute_changed\n", get_id());
	m_ch->send(msg);
	return OP_CONTINUE;
}

void sensor_listener_proxy::update_event(std::shared_ptr<ipc::message> msg)
{
	/* TODO: check axis orientation */
	msg->header()->type = CMD_LISTENER_EVENT;
	msg->header()->err = OP_SUCCESS;

	m_ch->send(msg);
}

void sensor_listener_proxy::update_accuracy(std::shared_ptr<ipc::message> msg)
{
	sensor_data_t *data = reinterpret_cast<sensor_data_t *>(msg->body());

	if (data->accuracy == m_last_accuracy)
		return;

	m_last_accuracy = data->accuracy;

	sensor_data_t acc_data = {0, };
	acc_data.accuracy = m_last_accuracy;

	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	acc_data.timestamp = ((unsigned long long)(ts.tv_sec)*1000000000LL + ts.tv_nsec) / 1000;

	auto acc_msg = ipc::message::create();

	retm_if(!acc_msg, "Failed to allocate memory");

	acc_msg->header()->type = CMD_LISTENER_ACC_EVENT;
	acc_msg->header()->err = OP_SUCCESS;
	acc_msg->enclose(&acc_data, sizeof(acc_data));

	m_ch->send(acc_msg);
}

int sensor_listener_proxy::start(bool policy)
{
	int ret;
	sensor_handler *sensor = m_manager->get_sensor(m_uri);
	retv_if(!sensor, -EINVAL);
	retvm_if(m_started && !policy, OP_SUCCESS, "Sensor is already started");

	_D("Listener[%d] try to start", get_id());

	ret = sensor->start(this);
	retv_if(ret < 0, OP_ERROR);

	/* m_started is changed only when it is explicitly called by user,
	 * not automatically determined by any pause policy. */
	if (policy)
		return OP_SUCCESS;

	m_started = true;
	return OP_SUCCESS;
}

int sensor_listener_proxy::stop(bool policy)
{
	sensor_handler *sensor = m_manager->get_sensor(m_uri);
	retv_if(!sensor, -EINVAL);
	retvm_if(!m_started && !policy, OP_SUCCESS, "Sensor is already stopped");

	_D("Listener[%d] try to stop", get_id());

	int ret = sensor->stop(this);
	retv_if(ret < 0, OP_ERROR);

	/* attributes and m_started are changed only when it is explicitly called by user,
	 * not automatically determined by any policy. */
	if (policy)
		return OP_SUCCESS;

	/* unset attributes */
	delete_batch_latency();

	m_started = false;
	return OP_SUCCESS;
}

int sensor_listener_proxy::set_interval(int32_t interval)
{
	sensor_handler *sensor = m_manager->get_sensor(m_uri);
	retv_if(!sensor, -EINVAL);

	_D("Listener[%d] try to set interval[%d]", get_id(), interval);

	int ret = sensor->set_interval(this, interval);
	apply_sensor_handler_need_to_notify_attribute_changed(sensor);

	return ret;
}

int sensor_listener_proxy::get_interval(int32_t& interval)
{
	sensor_handler *sensor = m_manager->get_sensor(m_uri);
	retv_if(!sensor, -EINVAL);

	_D("Listener[%d] try to get interval", get_id());

	return sensor->get_interval(this, interval);
}

int sensor_listener_proxy::set_max_batch_latency(int32_t max_batch_latency)
{
	sensor_handler *sensor = m_manager->get_sensor(m_uri);
	retv_if(!sensor, -EINVAL);

	_D("Listener[%d] try to set max batch latency[%d]", get_id(), max_batch_latency);
	int ret = sensor->set_batch_latency(this, max_batch_latency);
	apply_sensor_handler_need_to_notify_attribute_changed(sensor);

	return ret;
}

int sensor_listener_proxy::get_max_batch_latency(int32_t& max_batch_latency)
{
	sensor_handler *sensor = m_manager->get_sensor(m_uri);
	retv_if(!sensor, -EINVAL);

	_D("Listener[%d] try to get max batch latency", get_id());

	return sensor->get_batch_latency(this, max_batch_latency);
}

int sensor_listener_proxy::delete_batch_latency(void)
{
	sensor_handler *sensor = m_manager->get_sensor(m_uri);
	retv_if(!sensor, -EINVAL);

	_I("Listener[%d] try to delete batch latency", get_id());

	return sensor->delete_batch_latency(this);
}

int sensor_listener_proxy::set_passive_mode(bool passive)
{
	/* TODO: passive mode */
	m_passive = passive;
	return OP_SUCCESS;
}

int sensor_listener_proxy::set_attribute(int32_t attribute, int32_t value)
{
	sensor_handler *sensor = m_manager->get_sensor(m_uri);
	retv_if(!sensor, -EINVAL);

	_D("Listener[%d] try to set attribute[%d, %d]", get_id(), attribute, value);

	if (attribute == SENSORD_ATTRIBUTE_PAUSE_POLICY) {
		if (m_pause_policy != value) {
			m_pause_policy = value;
			set_need_to_notify_attribute_changed(true);
		}
		return OP_SUCCESS;
	} else if (attribute == SENSORD_ATTRIBUTE_AXIS_ORIENTATION) {
		if (m_axis_orientation != value) {
			m_axis_orientation = value;
			set_need_to_notify_attribute_changed(true);
		}
		return OP_SUCCESS;
	} else if (attribute == SENSORD_ATTRIBUTE_FLUSH) {
		return flush();
	}

	int ret = sensor->set_attribute(this, attribute, value);
	apply_sensor_handler_need_to_notify_attribute_changed(sensor);

	return ret;
}

int sensor_listener_proxy::get_attribute(int32_t attribute, int32_t *value)
{
	sensor_handler *sensor = m_manager->get_sensor(m_uri);
	retv_if(!sensor, -EINVAL);

	_D("Listener[%d] try to get attribute[%d] int", get_id(), attribute);

	if (attribute == SENSORD_ATTRIBUTE_PAUSE_POLICY) {
		*value = m_pause_policy;
		return OP_SUCCESS;
	} else if (attribute == SENSORD_ATTRIBUTE_AXIS_ORIENTATION) {
		*value = m_axis_orientation;
		return OP_SUCCESS;
	} else if (attribute == SENSORD_ATTRIBUTE_FLUSH) {
		return -EINVAL;
	}

	return sensor->get_attribute(attribute, value);
}

int sensor_listener_proxy::set_attribute(int32_t attribute, const char *value, int len)
{
	sensor_handler *sensor = m_manager->get_sensor(m_uri);
	retv_if(!sensor, -EINVAL);

	_D("Listener[%d] try to set string attribute[%d], len[%d]", get_id(), attribute, len);

	int ret = sensor->set_attribute(this, attribute, value, len);
	apply_sensor_handler_need_to_notify_attribute_changed(sensor);

	return ret;
}

int sensor_listener_proxy::get_attribute(int32_t attribute, char **value, int *len)
{
	sensor_handler *sensor = m_manager->get_sensor(m_uri);
	retv_if(!sensor, -EINVAL);

	_D("Listener[%d] try to get attribute str[%d]", get_id(), attribute);

	return sensor->get_attribute(attribute, value, len);
}

int sensor_listener_proxy::flush(void)
{
	sensor_handler *sensor = m_manager->get_sensor(m_uri);
	retv_if(!sensor, -EINVAL);

	return sensor->flush(this);
}

int sensor_listener_proxy::get_data(sensor_data_t **data, int *len)
{
	sensor_handler *sensor = m_manager->get_sensor(m_uri);
	retv_if(!sensor, -EINVAL);

	return sensor->get_cache(data, len);
}

std::string sensor_listener_proxy::get_required_privileges(void)
{
	sensor_handler *sensor = m_manager->get_sensor(m_uri);
	retv_if(!sensor, "");

	sensor_info info = sensor->get_sensor_info();
	return info.get_privilege();
}

void sensor_listener_proxy::on_policy_changed(int policy, int value)
{
	ret_if(m_started == false);
	ret_if(policy != SENSORD_ATTRIBUTE_PAUSE_POLICY);
	ret_if(m_pause_policy == SENSORD_PAUSE_NONE);

	_D("power_save_state[%d], listener[%d] pause policy[%d]",
			value, get_id(), m_pause_policy);

	if (value & m_pause_policy)
		stop(true);
	if (!(value & m_pause_policy))
		start(true);
}

bool sensor_listener_proxy::notify_attribute_changed(int32_t attribute, int32_t value)
{
	sensor_handler *sensor = m_manager->get_sensor(m_uri);
	retv_if(!sensor, -EINVAL);

	return sensor->notify_attribute_changed(m_id, attribute, value);
}

bool sensor_listener_proxy::notify_attribute_changed(int attribute, const char *value, int len)
{
	sensor_handler *sensor = m_manager->get_sensor(m_uri);
	retv_if(!sensor, -EINVAL);

	return sensor->notify_attribute_changed(m_id, attribute, value, len);
}

bool sensor_listener_proxy::need_to_notify_attribute_changed()
{
	return m_need_to_notify_attribute_changed;
}

void sensor_listener_proxy::set_need_to_notify_attribute_changed(bool value)
{
	m_need_to_notify_attribute_changed = value;
}

void sensor_listener_proxy::apply_sensor_handler_need_to_notify_attribute_changed(sensor_handler *handler)
{
	set_need_to_notify_attribute_changed(handler->need_to_notify_attribute_changed());
	handler->set_need_to_notify_attribute_changed(false);
}
