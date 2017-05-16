/*
 * sensord
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#ifndef __SENSOR_INTERNAL_H__
#define __SENSOR_INTERNAL_H__

#ifndef API
#define API __attribute__((visibility("default")))
#endif

#include <stdbool.h>
#include <sys/types.h>

/*header for common sensor type*/
#include <sensor_types.h>
#include <sensor_deprecated.h>
#include <sensor_internal_deprecated.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*sensor_cb_t)(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data);
typedef void (*sensorhub_cb_t)(sensor_t sensor, unsigned int event_type, sensorhub_data_t *data, void *user_data);
typedef void (*sensor_accuracy_changed_cb_t) (sensor_t sensor, unsigned long long timestamp, int accuracy, void *user_data);

/**
 * @brief Get the list of available sensors of a certain type,  use ALL_SENSOR to get all the sensors.
 *
 * @param[in] type the type of sensors requested.
 * @param[out] list the list of sensors matching the asked type,  the caller should explicitly free this list.
 * @param[out] sensor count the count of sensors contained in the list.
 * @return true on success, otherwise false.
 */
bool sensord_get_sensor_list(sensor_type_t type, sensor_t **list, int *sensor_count);

/**
 * @brief Get the default sensor for a given type.
 *
 * @param[in] type the type of a sensor requested.
 * @return the default sensor matching the asked type on success, otherwise NULL.
 */
sensor_t sensord_get_sensor(sensor_type_t type);

/**
 * @brief Get the list of available sensors of a certain type,  use ALL_SENSOR to get all the sensors.
 *
 * @param[in] type the type of sensors requested.
 * @param[out] list the list of sensors matching the asked type,  the caller should explicitly free this list.
 * @param[out] sensor count the count of sensors contained in the list.
 * @return 0 on success, otherwise a negative error value
 * @retval 0 Successful
 * @retval -EPERM Operation not permitted
 * @retval -EACCES Permission denied
 * @retval -ENODATA NO sensor available
 */
int sensord_get_sensors(sensor_type_t type, sensor_t **list, int *sensor_count);

/**
 * @brief Get the default sensor for a given type.
 *
 * @param[in] type the type of a sensor requested.
 * @param[out] a sensor matching the asked type.
 * @return 0 on success, otherwise a negative error value
 * @retval 0 Successful
 * @retval -EPERM Operation not permitted
 * @retval -EACCES Permission denied
 */
int sensord_get_default_sensor(sensor_type_t type, sensor_t *sensor);

/**
 * @brief Get the type of this sensor.
 *
 * @param[in] sensor a sensor to get type.
 * @param[out] type the type of this sensor.
 * @return return true on success, otherwise false.
 */
bool sensord_get_type(sensor_t sensor, sensor_type_t *type);

/**
 * @brief Get the URI string of this sensor.
 *
 * @param[in] sensor a sensor to get uri.
 * @return the name string of this sensor on success, otherwise NULL.
 */
const char* sensord_get_uri(sensor_t sensor);

/**
 * @brief Get the name string of this sensor.
 *
 * @param[in] sensor a sensor to get name.
 * @return the name string of this sensor on success, otherwise NULL.
 */
const char* sensord_get_name(sensor_t sensor);

/**
 * @brief Get the vendor string of this sensor.
 *
 * @param[in] sensor a sensor to get vendor.
 * @return the vendor string of this sensor on success, otherwise NULL.
 */
const char* sensord_get_vendor(sensor_t sensor);

/**
 * @brief Get the privilege of this sensor.
 *
 * @param[in] sensor a sensor to get privilege.
 * @param[out] privilege the privilege of this sensor.
 * @return true on success, otherwise false.
 */
bool sensord_get_privilege(sensor_t sensor, sensor_privilege_t *privilege);

/**
 * @brief Get the minimum range of this sensor in the sensor's unit.
 *
 * @param[in] sensor a sensor to get minimum range.
 * @param[out] min_range the minimum range of this sensor in the sensor's unit.
 * @return true on success, otherwise false.
 */
bool sensord_get_min_range(sensor_t sensor, float *min_range);

/**
 * @brief Get the maximum range of this sensor in the sensor's unit.
 *
 * @param[in] sensor a sensor to get maximum range.
 * @param[out] max_range the maximum range of this sensor in the sensor's unit.
 * @return true on success, otherwise false.
 */
bool sensord_get_max_range(sensor_t sensor, float *max_range);

/**
 * @brief Get the resolution of this sensor in the sensor's unit.
 *
 * @param[in] sensor a sensor to get resolution.
 * @param[out] resolution the resolution of this sensor in the sensor's unit.
 * @return true on success, otherwise false.
 */
bool sensord_get_resolution(sensor_t sensor, float *resolution);

/**
 * @brief Get the minimum interval allowed between two events in microsecond or zero if this sensor only returns a value when the data it's measuring changes.
 *
 * @param[in] sensor a sensor to get minimum interval.
 * @param[out] min_interval the minimum interval of this sensor.
 * @return true on success, otherwise false.
 */
bool sensord_get_min_interval(sensor_t sensor, int *min_interval);

/**
 * @brief Get the number of events reserved for this sensor in the batch mode FIFO.
 *
 * @param[in] sensor a sensor to get the number of fifo count
 * @param[out] fifo_count the number of events reserved for this sensor in the batch mode FIFO
 * @return true on success, otherwise false
 */
bool sensord_get_fifo_count(sensor_t sensor, int *fifo_count);

/**
 * @brief Get the maximum number of events of this sensor that could be batched. If this value is zero it indicates that batch mode is not supported for this sensor.
 *
 * @param[in] sensor a sensor to the maximum number of events that could be batched.
 * @param[out] max_batch_count the maximum number of events of this sensor that could be batched.
 * @return true on success, otherwise false.
 */
bool sensord_get_max_batch_count(sensor_t sensor, int *max_batch_count);

/**
 * @brief Get the supported event types of this sensor.
 *
 * @param[in] sensor a sensor to get the supported event types.
 * @param[out] event_types the array containing supported event types of this sensor, the caller should explicitly free this array.
 * @param[out] count the count of the supported event types of this sensor.
 * @return true on success, otherwise false.
 */
bool sensord_get_supported_event_types(sensor_t sensor, unsigned int **event_types, int *count);

/**
 * @brief Check a given event type is supporeted by this sensor.
 *
 * @param[in] sensor a sensor to check a given event type is supporeted.
 * @param[out] event_type an event type to be checked whether supported or not.
 * @param[out] supported whether a given event is supported or not in this sensor.
 * @return true on success, otherwise false.
 */
bool sensord_is_supported_event_type(sensor_t sensor, unsigned int event_type, bool *supported);

/**
 * @brief Check a wakeup supported or not by this sensor.
 *
 * @param[in] sensor a sensor to check a given event type is supporeted.
 * @return true on success, otherwise false.
 */
bool sensord_is_wakeup_supported(sensor_t sensor);

/**
 * @brief Connect a given sensor and get a handle of a given sensor.
 *
 * @param[in] sensor a sensor to connect
 * @return a handle of a given sensor on success, otherwise negative value
 */
int sensord_connect(sensor_t sensor);

/**
 * @brief Disconnect a given sensor.
 *
 * @param[in] handle a handle to disconnect.
 * @return true on success, otherwise false.
 */
bool sensord_disconnect(int handle);

/**
 * @brief Register a callback with a connected sensor for a given event_type. This callback will be called when a given event occurs in a connected sensor.
 *
 * @param[in] handle a handle represensting a connected sensor.
 * @param[in] event_type an event type  to register
 * @param[in] interval The desired interval between two consecutive events in microseconds. This is only a hint to the system so events may be received faster or slower than the specified interval.
 * 				     It can be one of SENSOR_INTERVAL_NORMAL,  SENSOR_INTERVAL_FASTEST or the interval in microseconds.
 * @param[in] max_batch_latency An event in the batch can be delayed by at most max_batch_latency microseconds. If this is set to zero, batch mode is disabled.
 * @param[in] cb a callback which is called when a given event occurs
 * @param[in] user_data the callback is called with user_data
 * @return true on success, otherwise false.
 */
bool sensord_register_event(int handle, unsigned int event_type, unsigned int interval, unsigned int max_batch_latency, sensor_cb_t cb, void *user_data);

/**
 * @brief Register a callback with a connected context sensor for a given event_type. This callback will be called when a given event occurs in a connected context sensor.
 *
 * @param[in] handle a handle represensting a connected context sensor.
 * @param[in] event_type an event type to register
 * @param[in] interval The desired interval between two consecutive events in microseconds. This is only a hint to the system so events may be received faster or slower than the specified interval.
 * 				      It can be one of SENSOR_INTERVAL_NORMAL,  SENSOR_INTERVAL_FASTEST or the interval in microseconds.
 * @param[in] max_batch_latency An event in the batch can be delayed by at most max_batch_latency microseconds. If this is set to zero, batch mode is disabled.
 * @param[in] cb a callback which is called when a given event occurs
 * @param[in] user_data the callback is called with user_data
 * @return true on success, otherwise false.
 */
bool sensord_register_hub_event(int handle, unsigned int event_type, unsigned int interval, unsigned int max_batch_latency, sensorhub_cb_t cb, void *user_data);

/**
 * @brief Unregister a event with a connected sensor.  After unregistering, that event will not be sent.
 *
 * @param[in] handle a handle represensting a connected sensor.
 * @param[in] event_type an event type to unregister.
 * @return true on success, otherwise false.
 */
bool sensord_unregister_event(int handle, unsigned int event_type);

/**
 * @brief Register a callback with a connected sensor. This callback will be called when the accuracy of a sensor has changed.
 *
 * @param[in] handle a handle represensting a connected sensor.
 * @param[in] cb a callback which is called when he accuracy of a sensor has changed.
 * @param[in] user_data the callback is called with user_data
 * @return true on success, otherwise false.
 */
bool sensord_register_accuracy_cb(int handle, sensor_accuracy_changed_cb_t cb, void *user_data);

/**
 * @brief Unregister a callback with a connected sensor.  After unregistering,  sensor_accuray_change_cb will not be called.
 *
 * @param[in] handle a handle represensting a connected sensor.
 * @return true on success, otherwise false.
 */
bool sensord_unregister_accuracy_cb(int handle);

/**
 * @brief Start listening events with a connected sensor.
 *
 * @param[in] handle a handle represensting a connected sensor.
 * @param[in] option either one of SENSOR_OPTION_DEFAULT and  SENSOR_OPTION_ALWAYS_ON.
 *				   with SENSOR_OPTION_DEFAULT, it stops to listening events when LCD is off or in power save mode.
 *				   with SENSOR_OPTION_ALWAYS_ON, it continues to listening events even when LCD is off or in power save mode.
 * @return true on success, otherwise false.
 */
bool sensord_start(int handle, int option);

/**
 * @brief Stop listening events with a connected sensor.
 *
 * @param[in] handle a handle represensting a connected sensor.
 * @return true on success, otherwise false.
 */
bool sensord_stop(int handle);

/**
 * @brief Change the interval of a specifed event type in a connected sensor.
 *
 * @param[in] handle a handle represensting a connected sensor.
 * @param[in] event_type an event type to change interval.
 * @param[in] interval The desired interval between two consecutive events in microseconds. This is only a hint to the system so events may be received faster or slower than the specified interval.
 * 				      It can be one of SENSOR_INTERVAL_NORMAL,  SENSOR_INTERVAL_FASTEST or the interval in microseconds.
 * @return true on success, otherwise false.
 */
bool sensord_change_event_interval(int handle, unsigned int event_type, unsigned int interval);

/**
 * @brief Change the max batch latency of a specifed event type in a connected sensor.
 *
 * @param[in] handle a handle represensting a connected sensor.
 * @param[in] event_type an event type to change max batch latency
 * @param[in] max_batch_latency an event in the batch can be delayed by at most max_batch_latency microseconds. If this is set to zero, batch mode is disabled.
 * @return true on success, otherwise false.
 */
bool sensord_change_event_max_batch_latency(int handle, unsigned int event_type, unsigned int max_batch_latency);

/**
 * @brief Change the option of a connected sensor.
 *
 * @param[in] handle a handle represensting a connected sensor.
 * @param[in] option either one of SENSOR_OPTION_DEFAULT and  SENSOR_OPTION_ALWAYS_ON.
 *				   with SENSOR_OPTION_DEFAULT, it stops to listening events when LCD is off or in power save mode.
 * 				   with SENSOR_OPTION_ALWAYS_ON, it continues to listening events even when LCD is off or in power save mode.
 * @return true on success, otherwise false.
 */
bool sensord_set_option(int handle, int option);

/*
 * @brief Set the attribute to a connected sensor
 *
 * @param[in] handle a handle represensting a connected sensor.
 * @param[in] attribute an attribute to change
 * @param[in] value an attribute value
 * @return 0 on success, otherwise a negative error value
 * @retval 0 Successful
 * @retval -EINVAL Invalid parameter
 * @retval -EPERM Operation not permitted
 */
int sensord_set_attribute_int(int handle, int attribute, int value);

/**
 * @brief Set the attribute to a connected sensor
 *
 * @param[in] handle a handle represensting a connected sensor.
 * @param[in] attribute an attribute to change
 * @param[in] value an attribute value
 * @param[in] value_len the length of value
 * @return 0 on success, otherwise a negative error value
 * @retval 0 Successful
 * @retval -EINVAL Invalid parameter
 * @retval -EPERM Operation not permitted
 */
int sensord_set_attribute_str(int handle, int attribute, const char *value, int len);

/**
 * @brief Send data to sensorhub
 *
 * @param[in] handle a handle represensting a connected context sensor.
 * @param[in] data it holds data to send to sensorhub
 * @param[in] data_len the length of data
 * @return true on success, otherwise false.
 */
bool sensord_send_sensorhub_data(int handle, const char *data, int data_len);
bool sensord_send_command(int handle, const char *command, int command_len);

/**
 * @brief get sensor data from a connected sensor
 *
 * @param[in] handle a handle represensting a connected context sensor.
 * @param[in] data_id it specifies data to get
 * @param[out] sensor_data data from connected sensor
 * @return true on success, otherwise false.
 */
bool sensord_get_data(int handle, unsigned int data_id, sensor_data_t* sensor_data);

/**
 * @brief flush sensor data from a connected sensor
 *
 * @param[in] handle a handle represensting a connected context sensor.
 * @return true on success, otherwise false.
 */
bool sensord_flush(int handle);

bool sensord_set_passive_mode(int handle, bool passive);


/* Sensor Internal API using URI */
int sensord_get_default_sensor_by_uri(const char *uri, sensor_t *sensor);
int sensord_get_sensors_by_uri(const char *uri, sensor_t **list, int *sensor_count);

typedef void (*sensord_added_cb)(const char *uri, void *user_data);
int sensord_add_sensor_added_cb(sensord_added_cb callback, void *user_data);
int sensord_remove_sensor_added_cb(sensord_added_cb callback);

typedef void (*sensord_removed_cb)(const char *uri, void *user_data);
int sensord_add_sensor_removed_cb(sensord_removed_cb callback, void *user_data);
int sensord_remove_sensor_removed_cb(sensord_removed_cb callback);

/* Sensor provider */
typedef void *sensord_provider_h;
int sensord_create_provider(const char *uri, sensord_provider_h *provider);
int sensord_destroy_provider(sensord_provider_h provider);
int sensord_add_provider(sensord_provider_h provider);
int sensord_remove_provider(sensord_provider_h provider);

int sensord_provider_set_name(sensord_provider_h provider, const char *name);
int sensord_provider_set_vendor(sensord_provider_h provider, const char *vendor);
int sensord_provider_set_range(sensord_provider_h provider, float min_range, float max_range);
int sensord_provider_set_resolution(sensord_provider_h provider, float resolution);

typedef void (*sensord_provider_start_cb)(sensord_provider_h provider, void *user_data);
int sensord_provider_set_start_cb(sensord_provider_h provider, sensord_provider_start_cb callback, void *user_data);

typedef void (*sensord_provider_stop_cb)(sensord_provider_h provider, void *user_data);
int sensord_provider_set_stop_cb(sensord_provider_h provider, sensord_provider_stop_cb callback, void *user_data);

typedef void (*sensord_provider_interval_changed_cb)(sensord_provider_h provider, unsigned int interval_ms, void *user_data);
int sensord_provider_set_interval_changed_cb(sensord_provider_h provider, sensord_provider_interval_changed_cb callback, void *user_data);

int sensord_provider_publish(sensord_provider_h provider, sensor_data_t data);

/* Deprecated */
typedef void (*sensor_external_command_cb_t)(int handle, const char* data, int data_cnt, void *user_data);
int sensord_external_connect(const char *key, sensor_external_command_cb_t cb, void *user_data);
bool sensord_external_disconnect(int handle);
bool sensord_external_post(int handle, unsigned long long timestamp, const float* data, int data_cnt);

/**
  * @}
 */

#ifdef __cplusplus
}
#endif

#endif
