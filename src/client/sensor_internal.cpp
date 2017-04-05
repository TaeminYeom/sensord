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

#include "sensor_internal.h"
#include <stdlib.h>
#include <sensor_log.h>

/*
 * TO-DO-LIST:
 * 1. restore session
 * 1.1 close socket
 * 1.2 listener : start, interval, batch latency, attributes
 * 2. power save option / lcd vconf : move to server
 * 3. clean up data
 * 4. thread-safe : ipc_client
 * 5. client-id
 * 6. cmd_hello
 * 7. stop : unset interval, batch
 * 8. get sensor list
 */

API int sensord_get_sensors(sensor_type_t type, sensor_t **list, int *count)
{
	/*
	 * 1. get sensor list()
	 * 2. if sensor list is empty, return -ENODATA
	 * 3. if id is -EACCESS, return -EACCESS (except ALL_SENSOR)
	 * 4. list : memory allocation
	 * 5. return list, count
	 */

	return OP_SUCCESS;
}

API int sensord_get_default_sensor(sensor_type_t type, sensor_t *sensor)
{
	/*
	 * 1. get sensor list()
	 * 2. if there is no sensor, return -ENODATA
	 * 3. if id is -EACCESS, return -EACCESS
	 * 4. if SENSOR_ALL, ???
	 * 5. return sensor
	 */

	return OP_SUCCESS;
}

API bool sensord_get_type(sensor_t sensor, sensor_type_t *type)
{
	/*
	 * 1. check parameter
	 * 2. if there is no sensor, return false
	 */

	return true;
}

API const char* sensord_get_name(sensor_t sensor)
{
	/*
	 * 1. if there is no sensor, return NULL
	 */

	return NULL;
}

API const char* sensord_get_vendor(sensor_t sensor)
{
	/*
	 * 1. if there is no sensor, return NULL
	 */

	return NULL;
}

API bool sensord_get_privilege(sensor_t sensor, sensor_privilege_t *privilege)
{
	/*
	 * 1. check parameter
	 * 2. if there is no sensor, return false
	 */

	return true;
}

API bool sensord_get_min_range(sensor_t sensor, float *min_range)
{
	/*
	 * 1. check parameter
	 * 2. if there is no sensor, return false
	 */

	return true;
}

API bool sensord_get_max_range(sensor_t sensor, float *max_range)
{
	/*
	 * 1. check parameter
	 * 2. if there is no sensor, return false
	 */

	return true;
}

API bool sensord_get_resolution(sensor_t sensor, float *resolution)
{
	/*
	 * 1. check parameter
	 * 2. if there is no sensor, return false
	 */

	return true;
}

API bool sensord_get_min_interval(sensor_t sensor, int *min_interval)
{
	/*
	 * 1. check parameter
	 * 2. if there is no sensor, return false
	 */

	return true;
}

API bool sensord_get_fifo_count(sensor_t sensor, int *fifo_count)
{
	/*
	 * 1. check parameter
	 * 2. if there is no sensor, return false
	 */

	return true;
}

API bool sensord_get_max_batch_count(sensor_t sensor, int *max_batch_count)
{
	/*
	 * 1. check parameter
	 * 2. if there is no sensor, return false
	 */

	return true;
}

API bool sensord_is_wakeup_supported(sensor_t sensor)
{
	/*
	 * 1. check parameter
	 * 2. if there is no sensor, return false
	 */

	return true;
}

API int sensord_connect(sensor_t sensor)
{
	/*
	 * 1. check parameter
	 * 2. if there is no sensor, return -EPERM
	 * 3. sensor is already registered(sensor), it doesn't need to connect server
	 * 4. create integer handle for only this client
	 * 5. if it is first connection(client), get client id from server
	 * 6. if it is first connection(client), start sensor event listener
	 * 7. sensor initialization : stop, pause_all
	 * 8. if it is first connection(client), send cmd hello
	 * 9. if cmd hello is failed and it is first connection(client) stop listener, remove id
	 */

	return 0;
}

API bool sensord_disconnect(int handle)
{
	/*
	 * 1. check parameter(check handle???)
	 * 2. check state of this handle
	 * 3. if it is on passive mode, unregister event and unset interval/latency
	 * 4. if it is not stop, stop it
	 * 5. if there is no active sensor(client), reset id & byebye and stop listener
	 */

	return true;
}

API bool sensord_register_event(int handle, unsigned int event_type,
		unsigned int interval, unsigned int max_batch_latency, sensor_cb_t cb, void *user_data)
{
	/*
	 * 1. check parameter
	 * 2. if interval is 0, default interval
	 * ** if cb is null, how to handle it?
	 * ** event type is deprecated
	 */

	return true;
}

API bool sensord_unregister_event(int handle, unsigned int event_type)
{
	/*
	 * 1. check parameter
	 * 2. check previous interval, latency, cb, user_data
	 * 3. if passive mode is true, set false
	 * 4. if ret is false, register event
	 */

	return true;
}

API bool sensord_register_accuracy_cb(int handle, sensor_accuracy_changed_cb_t cb, void *user_data)
{
	/*
	 * 1. check parameter
	 */

	return true;
}

API bool sensord_unregister_accuracy_cb(int handle)
{
	/*
	 * 1. check parameter
	 */

	return true;
}

API bool sensord_start(int handle, int option)
{
	/*
	 * 1. check parameter
	 * 2. pause = CONVERT_OPTION_PAUSE_POLICY(option)
	 * 3. start listener
	 */

	return true;
}

API bool sensord_stop(int handle)
{
	/*
	 * 1. check parameter
	 * 2. check sensor state, id
	 * 2.1. if sensor is already stopped, return true
	 * 3. stop listener
	 */

	return true;
}

API bool sensord_change_event_interval(int handle, unsigned int event_type, unsigned int interval)
{
	/*
	 * 1. check parameter
	 * 2. if interval is 0, default interval
	 * 3. if previous interval is lower than interval, return true
	 * 4. change interval
	 */

	return true;
}

API bool sensord_change_event_max_batch_latency(int handle, unsigned int event_type, unsigned int max_batch_latency)
{
	/*
	 * 1. check parameter
	 * 2. if previous interval is lower than interval, return true
	 * 3. change batch latency
	 */

	return true;
}

API bool sensord_set_option(int handle, int option)
{
	/*
	 * change option, always-on/power save option/lcd off/default
	 */

	return true;
}

API int sensord_set_attribute_int(int handle, int attribute, int value)
{
	/*
	 * 1. if ATTRIBUTE_PAUSE_POLICY
	 * 2. if ATTRIBUTE_AXIS_ORIENTATION
	 * 3. else attribute
	 */

	return OP_SUCCESS;
}

API int sensord_set_attribute_str(int handle, int attribute, const char *value, int len)
{
	/*
	 * 1. check parameter
	 * 2. if client id is invalid, return -EPERM
	 * 3. if there is other problems, return -EPERM
	 */

	return OP_SUCCESS;
}

API bool sensord_get_data(int handle, unsigned int data_id, sensor_data_t* sensor_data)
{
	/*
	 * 1. check parameter
	 * 2. check sensor state(is it really needed?)
	 * 3. get data
	 */

	return true;
}

API bool sensord_flush(int handle)
{
	/*
	 * 1. check parameter
	 * 2. check sensor state(is it really needed?)
	 * 3. flush sensor
	 */

	return true;
}

API bool sensord_set_passive_mode(int handle, bool passive)
{
	/* set passive mode*/

	return true;
}

API int sensord_external_connect(const char *key, sensor_external_command_cb_t cb, void *user_data)
{
	/*
	 * 1. check parameter
	 * 2. create handle in this client
	 * 3. first connection(client)
	 * 4. cmd_connect for external sensor with key
	 */
	retvm_if(!key, -EPERM, "Invalid key");
	return 0;
}

API bool sensord_external_disconnect(int handle)
{
	/*
	 * 1. check parameter
	 * 2. create handle in this client
	 * 3. first connection(client)
	 * 4. cmd_connect for external sensor with key
	 * 5. disconnect this handle
	 * 6. if there is no active sensor, remove client id and stop listener
	 */
	return true;
}

API bool sensord_external_post(int handle, unsigned long long timestamp, const float* data, int data_cnt)
{
	/*
	 * 1. check parameter
	 * 1.1 (data_cnt <= 0) || (data_cnt > POST_DATA_LEN_MAX)), return false
	 * 2. cmd_post
	 */

	return true;
}

/* deperecated */
API sensor_t sensord_get_sensor(sensor_type_t type)
{
	return NULL;
}

/* deprecated */
API bool sensord_get_sensor_list(sensor_type_t type, sensor_t **list, int *sensor_count)
{
	return (sensord_get_sensors(type, list, sensor_count) == OP_SUCCESS);
}

/* deprecated */
API bool sensord_register_hub_event(int handle, unsigned int event_type,
		unsigned int interval, unsigned int max_batch_latency, sensorhub_cb_t cb, void *user_data)
{
	return false;
}

/* deprecated */
API bool sensord_get_supported_event_types(sensor_t sensor, unsigned int **event_types, int *count)
{
	/*
	 * 1. check parameter
	 * 2. if there is no sensor, return false
	 * 3. memory allocation
	 */
	return true;
}

/* deprecated(BUT it is used in C-API....) */
API bool sensord_is_supported_event_type(sensor_t sensor, unsigned int event_type, bool *supported)
{
	/*
	 * 1. check parameter
	 * 2. if there is no sensor, return false
	 */
	return true;
}

/* deprecated */
API bool sensord_send_sensorhub_data(int handle, const char *data, int data_len)
{
	return (sensord_set_attribute_str(handle, 0, data, data_len) == OP_SUCCESS);
}

/* deprecated */
API bool sensord_send_command(int handle, const char *command, int command_len)
{
	return (sensord_set_attribute_str(handle, 0, command, command_len) == OP_SUCCESS);
}

