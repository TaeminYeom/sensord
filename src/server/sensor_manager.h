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

#ifndef __SENSOR_MANAGER_H__
#define __SENSOR_MANAGER_H__

#include <string>
#include <vector>
#include <unordered_map>

#include "event_loop.h"

#include "sensor_loader.h"

namespace sensor {

class sensor_manager {
public:
	sensor_manager(ipc::event_loop *loop);
	~sensor_manager();

	bool init(void);
	bool deinit(void);

private:
	ipc::event_loop *m_loop;
	sensor_loader m_loader;
};

}

#endif /* __SENSOR_MANAGER_H__ */
