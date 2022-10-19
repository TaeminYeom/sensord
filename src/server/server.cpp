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
#include <sys/stat.h>
#include <systemd/sd-daemon.h>
#include <sensor_log.h>
#include <command_types.h>
#include <ipc_server.h>

#include "sensor_manager.h"
#include "server_channel_handler.h"

#define MAX_CONFIG_PATH 255
#define CAL_CONFIG_PATH "/etc/sensor_cal.conf"
#define SET_CAL 1
//#define CAL_NODE_PATH "/sys/class/sensors/ssp_sensor/set_cal_data"

#define MAX_CONNECTION 1000

using namespace sensor;

ipc::event_loop server::m_loop;
std::atomic<bool> server::is_running(false);

server::server()
: m_server(NULL)
, m_manager(NULL)
, m_handler(NULL)
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

	init_calibration();
	init_server();

	is_running.store(true);
	sd_notify(0, "READY=1");

	_I("server initialization is complete");
	return true;
}

void server::deinit(void)
{
	m_server->close();
	m_manager->deinit();

	delete m_server;
	m_server = NULL;

	delete m_manager;
	m_manager = NULL;

	delete m_handler;
	m_handler = NULL;

	is_running.store(false);

	_I("server deinitialization is complete");
}

static void set_cal_data(const char *path)
{
	struct stat file_stat;

	if (lstat(path, &file_stat) != 0)
		return;

	if (!S_ISREG(file_stat.st_mode))
		return;

	FILE *fp = fopen(path, "w");
	retm_if(!fp, "There is no calibration file[%s]", path);

	fprintf(fp, "%d", SET_CAL);
	fclose(fp);

	_I("Succeeded to set calibration data");
}

void server::init_calibration(void)
{
	char path[MAX_CONFIG_PATH];

	FILE *fp = fopen(CAL_CONFIG_PATH, "r");
	retm_if(!fp, "There is no config file[%s]", CAL_CONFIG_PATH);

	while (!feof(fp)) {
		if (fgets(path, sizeof(path), fp) == NULL)
			break;
		set_cal_data(path);
	}

	fclose(fp);
}

void server::init_server(void)
{
	m_manager->init();

	/* TODO: setting socket option */
	m_server->set_option("max_connection", MAX_CONNECTION);
	m_server->set_option(SO_TYPE, SOCK_STREAM);
	m_server->bind(m_handler, &m_loop);
}
