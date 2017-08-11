/*
 * sensorctl
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

#pragma once /* __TESTER_H__ */

#include <sensor_internal.h>
#include <string>
#include <vector>

#include "sensor_manager.h"

#define TESTER_ARGC 3 /* e.g. {sensorctl, test, accelerometer} */

#define REGISTER_TESTER(name, tester_type) \
static tester_type tester(name);

class tester {
public:
	tester(const char *name);
	virtual ~tester() {}

	virtual bool setup(sensor_type_t type, int argc, char *argv[]) { return true; }
	virtual bool teardown(void) { return true; }
	virtual bool run(int argc, char *argv[]) = 0;

	const std::string& name() const { return m_name; }

private:
	std::string m_name;
};

class tester_manager : public sensor_manager {
public:
	static void register_tester(tester *test);

	tester_manager();
	virtual ~tester_manager();

	bool run(int argc, char *argv[]);
	void stop(void);

private:
	static std::vector<tester *> testers;

	tester *get_tester(const char *type);
	void usage(void);
};
