/*
 * sensord
 *
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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

#ifndef __SENSOR_POLICY_LISTENER_H__
#define __SENSOR_POLICY_LISTENER_H__

namespace sensor {
class sensor_policy_listener {
public:
	virtual ~sensor_policy_listener() {}

	virtual void on_policy_changed(int policy, int value) = 0;
};
}

#endif /* __SENSOR_POLICY_LISTENER_H__ */
