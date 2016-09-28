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

#include <sensor_callback_deliverer.h>
#include <sensor_event_listener.h>
#include <sensor_client_info.h>

sensor_callback_deliverer::sensor_callback_deliverer()
: m_running(false)
, m_callbacks(NULL)
{
}

sensor_callback_deliverer::~sensor_callback_deliverer()
{
}

bool sensor_callback_deliverer::start(void)
{
	if (is_running())
		return true;

	m_callbacks = g_async_queue_new();
	retvm_if(!m_callbacks, false, "Failed to allocated memory");

	m_deliverer = new(std::nothrow) std::thread(&sensor_callback_deliverer::run, this);

	if (!m_deliverer) {
		g_async_queue_unref(m_callbacks);
		_E("Failed to allocated memory");
		return false;
	}

	m_running = true;

	_I("Succeeded to start");

	return true;
}

bool sensor_callback_deliverer::stop(void)
{
	if (!is_running())
		return true;

	if (!terminate())
		return false;

	m_deliverer->join();
	delete m_deliverer;

	g_async_queue_unref(m_callbacks);

	_I("Succeeded to stop");

	return true;
}

bool sensor_callback_deliverer::is_running(void)
{
	return m_running.load();
}

bool sensor_callback_deliverer::push(client_callback_info *ci)
{
	if (!is_running())
		return false;

	retvm_if(!ci, false, "Invalid callback");

	g_async_queue_push(m_callbacks, ci);

	return true;
}

gboolean sensor_callback_deliverer::callback_dispatcher(gpointer data)
{
	bool ret;
	client_callback_info *ci = (client_callback_info *)data;

	ret = sensor_client_info::get_instance().is_event_active(ci->handle, ci->event_type, ci->event_id);

	if (!ret) {
		_W("Discard invalid callback cb(%#x)(%s, %#x, %#x) with id: %llu",
			ci->cb, get_event_name(ci->event_type), ci->sensor_data.get(),
			ci->user_data, ci->event_id);

		delete ci;
		return FALSE;
	}

	if (ci->accuracy_cb)
		ci->accuracy_cb(ci->sensor, ci->timestamp, ci->accuracy, ci->accuracy_user_data);

	if (ci->cb)
		ci->cb(ci->sensor, ci->event_type, (sensor_data_t *)ci->sensor_data.get(), ci->user_data);

	delete ci;

	/* To be called only once, it returns false */
	return FALSE;
}

void sensor_callback_deliverer::run(void)
{
	client_callback_info *ci;

	while (is_running()) {
		ci = static_cast<client_callback_info *>(g_async_queue_pop(m_callbacks));

		if (ci->handle == THREAD_TERMINATION) {
			m_running = false;
			delete ci;
			return;
		}

		deliver_to_main_loop(ci);
	}
}

bool sensor_callback_deliverer::terminate(void)
{
	client_callback_info *ci = new(std::nothrow) client_callback_info;
	retvm_if(!ci, false, "Failed to allocated memory");

	ci->handle = THREAD_TERMINATION;
	g_async_queue_push(m_callbacks, ci);

	return true;
}

void sensor_callback_deliverer::deliver_to_main_loop(client_callback_info *ci)
{
	/*
	 * Because callback function always returns FALSE,
	 * it is unnecessary to manage g_source id returned from g_idle_add().
	 */
	g_idle_add(callback_dispatcher, ci);
}
