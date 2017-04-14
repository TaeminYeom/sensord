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

#pragma once /* __SENSOR_ADAPTER_H__ */

#include <sensor_internal.h>

class sensor_info {
public:
	sensor_info(sensor_type_t _type, int _index, int _interval, int _batch_latency, int _powersave, sensor_cb_t _cb, void *_user_data)
	: type(_type)
	, index(_index)
	, interval(_interval)
	, batch_latency(_batch_latency)
	, powersave(_powersave)
	, cb(_cb)
	, user_data(_user_data)
	{ }

	sensor_type_t type;
	int index;
	int interval;
	int batch_latency;
	int powersave;
	sensor_cb_t cb;
	void *user_data;
};

class sensor_adapter {
public:
	static bool is_supported(sensor_type_t type);
	static int  get_count(sensor_type_t type);
	static bool get_handle(sensor_info info, int &handle);

	static bool start(sensor_info info, int &handle);
	static bool stop(sensor_info info, int handle);

	static bool change_interval(int handle, int interval);
	static bool change_batch_latency(int handle, int batch_latency);
	static bool change_powersave(int handle, int powersave);
	static bool set_attribute(int handle, int attribute, int value);
	static bool set_attribute(int handle, int attribute, char *value, int size);

	static bool get_data(int handle, sensor_type_t type, sensor_data_t &data);
	static bool flush(int handle);
};
