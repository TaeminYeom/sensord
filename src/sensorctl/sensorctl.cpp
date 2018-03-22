/*
 * sensorctl
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "log.h"
#include "sensor_manager.h"
#include "tester.h"
#include "injector.h"
#include "info.h"
#include "loopback.h"

static sensor_manager *manager;

static void usage(void)
{
	_N("usage: sensorctl <command> <sensor_type> [<args>]\n");

	_N("The sensorctl commands are:\n");
	_N("  test:   test sensor(s)\n");
	_N("  inject: inject the event to sensor\n");
	_N("  info:   show sensor infos\n");
}

static sensor_manager *create_manager(char *command)
{
	sensor_manager *manager = NULL;

	if (!strcmp(command, "test"))
		manager = new(std::nothrow) tester_manager;
	else if (!strcmp(command, "inject"))
		manager = new(std::nothrow) injector_manager;
	else if (!strcmp(command, "info"))
		manager = new(std::nothrow) info_manager;
	else if (!strcmp(command, "loopback"))
		manager = new(std::nothrow) loopback_manager;

	if (!manager) {
		_E("failed to allocate memory for manager\n");
		return NULL;
	}

	return manager;
}

static void destroy_manager(sensor_manager *manager)
{
	if (!manager)
		return;

	delete manager;
	manager = NULL;
}

static void signal_handler(int signo)
{
	_E("\nReceived SIGNAL(%d)\n", signo);

	manager->stop();
}

int main(int argc, char *argv[])
{
	signal(SIGINT,  signal_handler);
	signal(SIGHUP,  signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGABRT, signal_handler);
	signal(SIGTSTP, signal_handler);

	if (argc < 2) {
		usage();
		return EXIT_SUCCESS;
	}

	manager = create_manager(argv[1]);
	if (!manager) {
		usage();
		return EXIT_SUCCESS;
	}

	manager->run(argc, argv);

	destroy_manager(manager);

	return EXIT_SUCCESS;
}
