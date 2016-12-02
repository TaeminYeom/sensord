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

#pragma once // _INFO_MANAGER_H_

#include <sensor_internal.h>
#include "sensor_manager.h"

class info_manager : public sensor_manager {
public:
	info_manager() {}
	virtual ~info_manager() {}

	bool process(int argc, char *argv[]);
private:
	void sensor_info(sensor_t *sensors, int count);
	void usage(void);
};
