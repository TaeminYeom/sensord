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

#pragma once /* __SENSOR_MANAGER_H__ */

#include <sensor_internal.h>

class sensor_manager {
public:
	virtual ~sensor_manager();

	virtual bool run(int argc, char *argv[]);
	virtual void stop(void);

protected:
	sensor_type_t get_sensor_type(const char *name);

	void usage_sensors(void);
};
