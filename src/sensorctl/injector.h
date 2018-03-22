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

#pragma once /* __INJECTOR_H__ */

#include <sensor_internal.h>
#include <string>
#include <vector>

#include "sensor_manager.h"

#define INJECTOR_ARGC 4 /* e.g. {sensorctl, inject, wristup, conf} */

#define REGISTER_INJECTOR(sensor_type, event_name, injector_type) \
static injector_type injector(sensor_type, event_name);

class injector {
public:
	injector(sensor_type_t sensor_type, const char *event_name);
	virtual ~injector() {}

	virtual bool setup(void) { return true; }
	virtual bool teardown(void) { return true; }

	const std::string& name() const { return m_name; }
	sensor_type_t type() { return m_type; }

	virtual bool inject(int argc, char *argv[]) = 0;

private:
	sensor_type_t m_type;
	std::string m_name;
};

class injector_manager : public sensor_manager {
public:
	static void register_injector(injector *inject);

	injector_manager();
	virtual ~injector_manager();

	bool run(int argc, char *argv[]);

private:
	static std::vector<injector *> injectors;

	injector *get_injector(sensor_type_t type, const char *name);
	void usage(void);
};
