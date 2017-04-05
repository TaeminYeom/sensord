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

#include "server.h"

#include <unistd.h>
#include <systemd/sd-daemon.h>
#include <sensor_log.h>
#include <command_types.h>
#include <ipc_server.h>

#include "sensor_manager.h"
#include "server_channel_handler.h"

#define TIMEOUT_TERM 10

using namespace sensor;

ipc::event_loop server::m_loop;
std::atomic<bool> server::is_running(false);

server::server()
: m_server(NULL)
, m_manager(NULL)
, m_handler(NULL)
{
}

server::~server()
{
}

server &server::instance(void)
{
	static server inst;
	return inst;
}

void server::run(void)
{
	_I("Starting..");

	retm_if(is_running.load(), "Server is running");
	retm_if(!instance().init(), "Failed to initialize server");

	m_loop.run();
}

void server::stop(void)
{
	_I("Stopping..");

	retm_if(!is_running.load(), "Server is not running");

	m_loop.stop();
	instance().deinit();
}

bool server::init(void)
{
	m_server = new(std::nothrow) ipc::ipc_server(SENSOR_CHANNEL_PATH);
	retvm_if(!m_server, false, "Failed to allocate memory");

	m_manager = new(std::nothrow) sensor_manager(&m_loop);
	retvm_if(!m_manager, false, "Failed to allocate memory");

	m_handler = new(std::nothrow) server_channel_handler(m_manager);
	retvm_if(!m_handler, false, "Failed to allocate memory");

	init_server();
	init_termination();

	is_running.store(true);
	sd_notify(0, "READY=1");

	return true;
}

void server::deinit(void)
{
	m_manager->deinit();
	m_server->close();

	delete m_server;
	m_server = NULL;

	delete m_manager;
	m_manager = NULL;

	delete m_handler;
	m_handler = NULL;

	is_running.store(false);
}

void server::init_server(void)
{
	m_manager->init();

	/* TODO: setting socket option */
	m_server->set_option("max_connection", 1000);
	m_server->set_option(SO_TYPE, SOCK_STREAM);
	m_server->bind(m_handler, &m_loop);
}

static gboolean terminate(gpointer data)
{
	/* TODO: if there is no sensor, sensord will be terminated */

	return FALSE;
}

void server::init_termination(void)
{
	g_timeout_add_seconds(TIMEOUT_TERM, terminate, m_manager);
}
