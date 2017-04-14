/*
 * sensord
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
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

#include <errno.h>
#include <sensor_types.h>

#include <sensor_internal.h>
#include <sensor_internal_deprecated.h>

API int sensord_external_connect(const char *key, sensor_external_command_cb_t cb, void *user_data)
{
	return OP_ERROR;
}

API bool sensord_external_disconnect(int handle)
{
	return false;
}

API bool sensord_external_post(int handle, unsigned long long timestamp, const float* data, int data_cnt)
{
	return false;
}
