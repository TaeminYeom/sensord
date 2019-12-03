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

#ifndef __SENSOR_OBSERVER_H__
#define __SENSOR_OBSERVER_H__

#include <message.h>

namespace sensor {

class sensor_observer {
public:
	virtual ~sensor_observer() {}

	virtual int update(const char *uri, std::shared_ptr<ipc::message> msg) = 0;
};

}

#endif /* __SENSOR_OBSERVER_H__ */
