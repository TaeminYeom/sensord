/*
 * sensord
 *
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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

#include "sensor_reader.h"

#include <sensor_log.h>
#include <sensor_types.h>
#include <chrono>

using namespace sensor;

sensor_reader::sensor_reader()
: m_reader(NULL)
, m_event_loop(NULL)
, m_mutex()
, m_cond()
, m_loop(NULL)
, m_running(false)
{
	m_event_loop = new(std::nothrow) ipc::event_loop();
	m_reader = new(std::nothrow) std::thread(&sensor::sensor_reader::read_event, this);
	m_reader->detach();

	if (!m_running)
		wait_for_preparation();

	_I("Created");
}

sensor_reader::~sensor_reader()
{
	_I("Destroying..");
	retm_if(!m_reader, "Invalid reader");

	m_running = false;

	m_event_loop->stop();

	delete m_reader;
	m_reader = NULL;

	delete m_event_loop;
	m_event_loop = NULL;

	_I("Destroyed");
}

ipc::event_loop *sensor_reader::get_event_loop(void)
{
	retvm_if(!m_event_loop, NULL, "Invalid context");

	return m_event_loop;
}

void sensor_reader::wait_for_preparation(void)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_cond.wait_for(lock, std::chrono::seconds(1));
}

void sensor_reader::read_event(void)
{
	_I("RUN");
	m_loop = g_main_loop_new(g_main_context_new(), false);
	m_event_loop->set_mainloop(m_loop);

	m_cond.notify_one();
	m_running = true;

	if (!m_event_loop->run())
		_E("Failed to run event loop");
}
