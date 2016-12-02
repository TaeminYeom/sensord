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

#include <sensorctl_log.h>
#include "sensor_manager.h"
#include "tester_manager.h"
#include "injector_manager.h"
#include "info_manager.h"
#include "loopback_manager.h"

static void good_bye(void)
{
}

static void signal_handler(int signo)
{
	_E("\nReceived SIGNAL(%d)\n", signo);
	exit(EXIT_SUCCESS);

	return;
}

void usage(void)
{
	PRINT("usage: sensorctl <command> <sensor_type> [<args>]\n");

	PRINT("The sensorctl commands are:\n");
	PRINT("  test:   test sensor(s)\n");
	PRINT("  inject: inject the event to sensor\n");
	PRINT("  info:   show sensor infos\n");
	PRINT("  loopback: sensor loopback test\n");
}

sensor_manager *create_manager(int argc, char *argv[2])
{
	sensor_manager *manager = NULL;

	if (!strcmp(argv[1], "test"))
		manager = new(std::nothrow) tester_manager;
	if (!strcmp(argv[1], "inject"))
		manager = new(std::nothrow) injector_manager;
	if (!strcmp(argv[1], "info"))
		manager = new(std::nothrow) info_manager;
	if (!strcmp(argv[1], "loopback"))
		manager = new(std::nothrow) loopback_manager;

	if (!manager) {
		_E("failed to allocate memory for manager");
		return NULL;
	}

	return manager;
}

int main(int argc, char *argv[])
{
	atexit(good_bye);

	signal(SIGINT,  signal_handler);
	signal(SIGHUP,  signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGABRT, signal_handler);

	if (argc < 2) {
		usage();
		return 0;
	}

	sensor_manager *manager = create_manager(argc, argv);
	if (!manager) {
		usage();
		return 0;
	}

	manager->process(argc, argv);

	return 0;
}
