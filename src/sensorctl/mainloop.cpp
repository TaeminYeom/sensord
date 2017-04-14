/*
 * sensorctl
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

#include "mainloop.h"

void mainloop::run(void)
{
	instance().start_loop();
}

void mainloop::stop(void)
{
	instance().stop_loop();
}

bool mainloop::is_running(void)
{
	return instance().is_loop_running();
}

mainloop& mainloop::instance(void)
{
	static mainloop loop;
	return loop;
}

void mainloop::start_loop(void)
{
	if (is_loop_running())
		return;

	m_mainloop = g_main_loop_new(NULL, false);
	m_running.store(true);

	g_main_loop_run(m_mainloop);
}

void mainloop::stop_loop(void)
{
	if (!is_loop_running())
		return;

	g_main_loop_quit(m_mainloop);
	g_main_loop_unref(m_mainloop);
	m_mainloop = NULL;
	m_running.store(false);
}

bool mainloop::is_loop_running(void)
{
	return m_running.load();
}

