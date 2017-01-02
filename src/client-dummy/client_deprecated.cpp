/*
 * sensord
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

#include <sensor_internal_deprecated.h>

#ifndef API
#define API __attribute__((visibility("default")))
#endif

API int sf_connect(sensor_type_t sensor_type)
{
	return OP_ERROR;
}

API int sf_disconnect(int handle)
{
	return OP_ERROR;
}

API int sf_start(int handle, int option)
{
	return OP_ERROR;
}

API int sf_stop(int handle)
{
	return OP_ERROR;
}

API int sf_register_event(int handle, unsigned int event_type, event_condition_t *event_condition, sensor_callback_func_t cb, void *user_data)
{
	return OP_ERROR;
}

API int sf_unregister_event(int handle, unsigned int event_type)
{
	return OP_ERROR;
}

API int sf_change_event_condition(int handle, unsigned int event_type, event_condition_t *event_condition)
{
	return OP_ERROR;
}

API int sf_change_sensor_option(int handle, int option)
{
	return OP_ERROR;
}

API int sf_send_sensorhub_data(int handle, const char* data, int data_len)
{
	return OP_ERROR;
}

API int sf_get_data(int handle, unsigned int data_id, sensor_data_t* sensor_data)
{
	return OP_ERROR;
}

API int sf_check_rotation(unsigned long *rotation)
{
	return OP_ERROR;
}

API int sf_is_sensor_event_available(sensor_type_t sensor_type, unsigned int event_type)
{
	return OP_ERROR;
}
