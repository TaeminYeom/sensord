/*
 * sensord
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#include <unistd.h>
#include <sensor_log.h>
#include <dbus_util.h>
#include <new>
#include <csignal>

#include "server.h"

#define NEW_FAIL_LIMIT 3

using namespace sensor;

static void on_signal(int signum)
{
	_W("Received SIGNAL(%d : %s)", signum, strsignal(signum));
	server::stop();
}

static void on_new_failed(void)
{
	static unsigned fail_count = 0;
	_E("Failed to allocate memory");

	fail_count += 1;
	if (fail_count >= NEW_FAIL_LIMIT) {
		raise(SIGTERM);
		return;
	}

	usleep(100000);
}

int main(int argc, char *argv[])
{
	_I("Started");
	std::signal(SIGINT, on_signal);
	std::signal(SIGHUP, on_signal);
	std::signal(SIGTERM, on_signal);
	std::signal(SIGQUIT, on_signal);
	std::signal(SIGABRT, on_signal);
	std::signal(SIGCHLD, SIG_IGN);
	std::signal(SIGPIPE, SIG_IGN);

	std::set_new_handler(on_new_failed);

	init_dbus();

	server::run();

	fini_dbus();

	_I("Stopped");

	return 0;
}
