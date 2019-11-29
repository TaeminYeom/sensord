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

API int sensord_get_sensors(sensor_type_t type, sensor_t **list, int *sensor_count)
{
	return -ENODATA;
}

API int sensord_get_default_sensor(sensor_type_t type, sensor_t *sensor)
{
	return -ENODATA;
}

API bool sensord_get_sensor_list(sensor_type_t type, sensor_t **list, int *sensor_count)
{
	return false;
}

API sensor_t sensord_get_sensor(sensor_type_t type)
{
	return NULL;
}

API bool sensord_get_type(sensor_t sensor, sensor_type_t *type)
{
	return false;
}

API const char* sensord_get_uri(sensor_t sensor)
{
	return NULL;
}

API const char* sensord_get_name(sensor_t sensor)
{
	return NULL;
}

API const char* sensord_get_vendor(sensor_t sensor)
{
	return NULL;
}

API bool sensord_get_privilege(sensor_t sensor, sensor_privilege_t *privilege)
{
	return false;
}

API bool sensord_get_min_range(sensor_t sensor, float *min_range)
{
	return false;
}

API bool sensord_get_max_range(sensor_t sensor, float *max_range)
{
	return false;
}

API bool sensord_get_resolution(sensor_t sensor, float *resolution)
{
	return false;
}

API bool sensord_get_min_interval(sensor_t sensor, int *min_interval)
{
	return false;
}

API bool sensord_get_fifo_count(sensor_t sensor, int *fifo_count)
{
	return false;
}

API bool sensord_get_max_batch_count(sensor_t sensor, int *max_batch_count)
{
	return false;
}

API bool sensord_get_supported_event_types(sensor_t sensor, unsigned int **event_types, int *count)
{
	return false;
}

API bool sensord_is_supported_event_type(sensor_t sensor, unsigned int event_type, bool *supported)
{
	return false;
}

API bool sensord_is_wakeup_supported(sensor_t sensor)
{
	return false;
}

API int sensord_connect(sensor_t sensor)
{
	return OP_ERROR;
}

API bool sensord_disconnect(int handle)
{
	return false;
}

API bool sensord_register_event(int handle, unsigned int event_type, unsigned int interval, unsigned int max_batch_latency, sensor_cb_t cb, void *user_data)
{
	return false;
}

API bool sensord_unregister_event(int handle, unsigned int event_type)
{
	return false;
}

API bool sensord_register_events(int handle, unsigned int event_type, unsigned int max_batch_latency, sensor_events_cb_t cb, void *user_data)
{
	return false;
}

API bool sensord_unregister_events(int handle, unsigned int event_type)
{
	return false;
}

API bool sensord_register_accuracy_cb(int handle, sensor_accuracy_changed_cb_t cb, void *user_data)
{
	return false;
}

API bool sensord_unregister_accuracy_cb(int handle)
{
	return false;
}

API bool sensord_register_attribute_int_changed_cb(int handle, sensor_attribute_int_changed_cb_t cb, void *user_data)
{
	return false;
}

API bool sensord_unregister_attribute_int_changed_cb(int handle)
{
	return false;
}

API bool sensord_register_attribute_str_changed_cb(int handle, sensor_attribute_str_changed_cb_t cb, void *user_data)
{
	return false;
}

API bool sensord_unregister_attribute_str_changed_cb(int handle)
{
	return false;
}

API bool sensord_start(int handle, int option)
{
	return false;
}

API bool sensord_stop(int handle)
{
	return false;
}

API bool sensord_change_event_interval(int handle, unsigned int event_type, unsigned int interval)
{
	return false;
}

API bool sensord_change_event_max_batch_latency(int handle, unsigned int event_type, unsigned int max_batch_latency)
{
	return false;
}

API bool sensord_set_option(int handle, int option)
{
	return false;
}

API int sensord_set_attribute_int(int handle, int attribute, int value)
{
	return OP_ERROR;
}

API int sensord_get_attribute_int(int handle, int attribute, int* value)
{
	return OP_ERROR;
}

API int sensord_set_attribute_str(int handle, int attribute, const char *value, int len)
{
	return OP_ERROR;
}

API int sensord_get_attribute_str(int handle, int attribute, char **value, int *len)
{
	return OP_ERROR;
}

API bool sensord_send_sensorhub_data(int handle, const char *data, int data_len)
{
	return false;
}

API bool sensord_send_command(int handle, const char *command, int command_len)
{
	return false;
}

API bool sensord_get_data(int handle, unsigned int data_id, sensor_data_t* sensor_data)
{
	return false;
}

API bool sensord_get_data_list(int handle, unsigned int data_id, sensor_data_t** sensor_data, int* count)
{
	return false;
}

API bool sensord_flush(int handle)
{
	return false;
}

API bool sensord_register_hub_event(int handle, unsigned int event_type, unsigned int interval, unsigned int max_batch_latency, sensorhub_cb_t cb, void *user_data)
{
	return false;
}

API bool sensord_set_passive_mode(int handle, bool passive)
{
	return false;
}

/* Sensor Internal API using URI */
API int sensord_get_default_sensor_by_uri(const char *uri, sensor_t *sensor)
{
	return OP_ERROR;
}

API int sensord_get_sensors_by_uri(const char *uri, sensor_t **list, int *count)
{
	return OP_ERROR;
}

API int sensord_add_sensor_added_cb(sensord_added_cb callback, void *user_data)
{
	return OP_ERROR;
}

API int sensord_remove_sensor_added_cb(sensord_added_cb callback)
{
	return OP_ERROR;
}

API int sensord_add_sensor_removed_cb(sensord_removed_cb callback, void *user_data)
{
	return OP_ERROR;
}

API int sensord_remove_sensor_removed_cb(sensord_removed_cb callback)
{
	return OP_ERROR;
}

/* Sensor provider */
API int sensord_create_provider(const char *uri, sensord_provider_h *provider)
{
	return OP_ERROR;
}

API int sensord_destroy_provider(sensord_provider_h provider)
{
	return OP_ERROR;
}

API int sensord_add_provider(sensord_provider_h provider)
{
	return OP_ERROR;
}

API int sensord_remove_provider(sensord_provider_h provider)
{
	return OP_ERROR;
}

API int sensord_provider_set_name(sensord_provider_h provider, const char *name)
{
	return OP_ERROR;
}

API int sensord_provider_set_vendor(sensord_provider_h provider, const char *vendor)
{
	return OP_ERROR;
}

API int sensord_provider_set_range(sensord_provider_h provider, float min_range, float max_range)
{
	return OP_ERROR;
}

API int sensord_provider_set_resolution(sensord_provider_h provider, float resolution)
{
	return OP_ERROR;
}

API int sensord_provider_set_start_cb(sensord_provider_h provider, sensord_provider_start_cb callback, void *user_data)
{
	return OP_ERROR;
}

API int sensord_provider_set_stop_cb(sensord_provider_h provider, sensord_provider_stop_cb callback, void *user_data)
{
	return OP_ERROR;
}

API int sensord_provider_set_interval_changed_cb(sensord_provider_h provider, sensord_provider_interval_changed_cb callback, void *user_data)
{
	return OP_ERROR;
}

API int sensord_provider_publish(sensord_provider_h provider, sensor_data_t data)
{
	return OP_ERROR;
}

API int sensord_provider_publish_events(sensord_provider_h provider, sensor_data_t events[], int count)
{
	return OP_ERROR;
}
