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

#pragma once // _INJECT_MANAGER_H_

#include <sensor_internal.h>
#include <vector>
#include "injector.h"
#include "sensor_manager.h"

#define REGISTER_INJECTOR(sensor_type, sensor_name, injector_type) \
static void __attribute__((constructor)) reg_injector(void) \
{ \
	struct injector_info info; \
	info.type = (sensor_type); \
	info.name = (sensor_name); \
	info.injector = new(std::nothrow) (injector_type)(); \
	if (!info.injector) { \
		_E("ERROR: Failed to allocate memory(%s)", #injector_type); \
		return; \
	} \
	injector_manager::register_injector(info); \
}

struct injector_info {
	sensor_type_t type;
	const char *name;
	injector_interface *injector;
};

class injector_manager : public sensor_manager {
public:
	injector_manager();
	virtual ~injector_manager();

	bool process(int argc, char *argv[]);

	static bool register_injector(injector_info info);
private:
	injector_interface *get_injector(sensor_type_t type, const char *name);
	void usage(void);
};
