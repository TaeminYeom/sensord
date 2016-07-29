/*
 * sensord
 *
 * Copyright (c) 2014-15 Samsung Electronics Co., Ltd.
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
#include <time.h>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <sensor_internal.h>
#include <stdbool.h>
#include <sensor_common.h>
#include <unistd.h>
#include <string.h>
#include "check-sensor.h"

void printpollinglogs(sensor_type_t type,sensor_data_t data)
{
	switch(type) {
	case(ACCELEROMETER_SENSOR):
		printf("Accelerometer [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data.timestamp, data.values[0], data.values[1], data.values[2]);
		break;
	case(AUTO_ROTATION_SENSOR):
		printf("Auto Rotation [%lld] [%6.6f]\n\n", data.timestamp, data.values[0]);
		break;
	case(GYROSCOPE_SENSOR):
		printf("Gyroscope [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data.timestamp, data.values[0], data.values[1], data.values[2]);
		break;
	case(PRESSURE_SENSOR):
		printf("Pressure [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data.timestamp, data.values[0], data.values[1], data.values[2]);
		break;
	case(GEOMAGNETIC_SENSOR):
		printf("Geomagnetic [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data.timestamp, data.values[0], data.values[1], data.values[2]);
		break;
	case(LIGHT_SENSOR):
		printf("Light [%lld] [%6.6f]\n\n", data.timestamp, data.values[0]);
		break;
	case(TEMPERATURE_SENSOR):
		printf("Temperature [%lld] [%6.6f]\n\n", data.timestamp, data.values[0]);
		break;
	case(PROXIMITY_SENSOR):
		printf("Proximity [%lld] [%6.6f]\n\n", data.timestamp, data.values[0]);
		break;
	case(ULTRAVIOLET_SENSOR):
		printf("Ultraviolet [%lld] [%6.6f]\n", data.timestamp, data.values[0]);
		break;
	case(BIO_LED_RED_SENSOR):
		printf("Bio_LED_Red [%lld] [%6.6f]\n", data.timestamp, data.values[0]);
		break;
	case(ORIENTATION_SENSOR):
		printf("Orientation [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data.timestamp, data.values[0], data.values[1], data.values[2]);
		break;
	case(TILT_SENSOR):
		printf("Tilt [%lld] [%6.6f] [%6.6f]\n\n", data.timestamp, data.values[0], data.values[1]);
		break;
	case(GRAVITY_SENSOR):
		printf("Gravity [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data.timestamp, data.values[0], data.values[1], data.values[2]);
		break;
	case(LINEAR_ACCEL_SENSOR):
		printf("Linear Acceleration [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data.timestamp, data.values[0], data.values[1], data.values[2]);
		break;
	case(ROTATION_VECTOR_SENSOR):
		printf("Rotation Vector [%lld] [%6.6f] [%6.6f] [%6.6f] [%6.6f]\n\n", data.timestamp, data.values[0], data.values[1], data.values[2], data.values[3]);
		break;
	case(GEOMAGNETIC_RV_SENSOR):
		printf("Geomagnetic Rv [%lld] [%6.6f] [%6.6f] [%6.6f] [%6.6f]\n\n", data.timestamp, data.values[0], data.values[1], data.values[2], data.values[3]);
		break;
	case(GYROSCOPE_RV_SENSOR):
		printf("Gaming Rv [%lld] [%6.6f] [%6.6f] [%6.6f] [%6.6f]\n\n", data.timestamp, data.values[0], data.values[1], data.values[2], data.values[3]);
		break;
	case(GYROSCOPE_UNCAL_SENSOR):
		printf("Gyroscope Uncal [%lld] [%6.6f] [%6.6f] [%6.6f] [%6.6f] [%6.6f] [%6.6f]\n\n", data.timestamp, data.values[0], data.values[1], data.values[2], data.values[3], data.values[4], data.values[5]);
		break;
	case(GESTURE_FACE_DOWN_SENSOR):
		printf("Face Down [%lld] [%6.6f]\n\n", data.timestamp, data.values[0]);
		break;
	default:
		return;
	}
}

int get_event(sensor_type_t sensor_type, char str[])
{
	switch (sensor_type) {
	case ACCELEROMETER_SENSOR:
		if (strcmp(str, "RAW_DATA_EVENT") == 0)
			return ACCELEROMETER_RAW_DATA_EVENT;
		break;
	case AUTO_ROTATION_SENSOR:
		if (strcmp(str, "CHANGE_STATE_EVENT") == 0)
			return AUTO_ROTATION_CHANGE_STATE_EVENT;
		break;
	case GYROSCOPE_SENSOR:
		if (strcmp(str, "RAW_DATA_EVENT") == 0)
			return GYROSCOPE_RAW_DATA_EVENT;
		break;
	case PRESSURE_SENSOR:
		if (strcmp(str, "RAW_DATA_EVENT") == 0)
			return PRESSURE_RAW_DATA_EVENT;
		break;
	case GEOMAGNETIC_SENSOR:
		if (strcmp(str, "RAW_DATA_EVENT") == 0)
			return GEOMAGNETIC_RAW_DATA_EVENT;
		break;
	case LIGHT_SENSOR:
		if (strcmp(str, "RAW_DATA_EVENT") == 0)
			return LIGHT_LUX_DATA_EVENT;
		break;
	case TEMPERATURE_SENSOR:
		if (strcmp(str, "RAW_DATA_EVENT") == 0)
			return TEMPERATURE_RAW_DATA_EVENT;
		break;
	case PROXIMITY_SENSOR:
		if (strcmp(str, "CHANGE_STATE_EVENT") == 0)
			return PROXIMITY_CHANGE_STATE_EVENT;
		break;
	case ULTRAVIOLET_SENSOR:
		if (strcmp(str, "RAW_DATA_EVENT") == 0)
			return ULTRAVIOLET_RAW_DATA_EVENT;
		break;
	case BIO_LED_RED_SENSOR:
		if (strcmp(str, "RAW_DATA_EVENT") == 0)
			return BIO_LED_RED_RAW_DATA_EVENT;
		break;
	case ORIENTATION_SENSOR:
		if (strcmp(str, "RAW_DATA_EVENT") == 0)
			return ORIENTATION_RAW_DATA_EVENT;
		break;
	case TILT_SENSOR:
		if (strcmp(str, "RAW_DATA_EVENT") == 0)
			return TILT_RAW_DATA_EVENT;
		break;
	case GRAVITY_SENSOR:
		if (strcmp(str, "RAW_DATA_EVENT") == 0)
			return GRAVITY_RAW_DATA_EVENT;
		break;
	case LINEAR_ACCEL_SENSOR:
		if (strcmp(str, "RAW_DATA_EVENT") == 0)
			return LINEAR_ACCEL_RAW_DATA_EVENT;
		break;
	case ROTATION_VECTOR_SENSOR:
		if (strcmp(str, "RAW_DATA_EVENT") == 0)
			return ROTATION_VECTOR_RAW_DATA_EVENT;
		break;
	case GEOMAGNETIC_RV_SENSOR:
		if (strcmp(str, "RAW_DATA_EVENT") == 0)
			return GEOMAGNETIC_RV_RAW_DATA_EVENT;
		break;
	case GYROSCOPE_RV_SENSOR:
		if (strcmp(str, "RAW_DATA_EVENT") == 0)
			return GAMING_RV_RAW_DATA_EVENT;
		break;
	case GYROSCOPE_UNCAL_SENSOR:
		if (strcmp(str, "RAW_DATA_EVENT") == 0)
			return GYROSCOPE_UNCAL_RAW_DATA_EVENT;
		break;
	case GESTURE_FACE_DOWN_SENSOR:
		if (strcmp(str, "RAW_DATA_EVENT") == 0)
			return FACE_DOWN_RAW_DATA_EVENT;
		break;

	default:
		return -1;
	}
	return -1;
}

void callback(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	sensor_type_t sensor_type = event_type >> 16;

	switch (sensor_type) {
	case ACCELEROMETER_SENSOR:
		printf("Accelerometer [%lld] [%6.6f] [%6.6f] [%6.6f]\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
		break;
	case AUTO_ROTATION_SENSOR:
		printf("Auto Rotation [%lld] [%6.6f]\n", data->timestamp, data->values[0]);
		break;
	case GYROSCOPE_SENSOR:
		printf("Gyroscope [%lld] [%6.6f] [%6.6f] [%6.6f]\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
		break;
	case PRESSURE_SENSOR:
		printf("Pressure [%lld] [%6.6f] [%6.6f] [%6.6f]\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
		break;
	case GEOMAGNETIC_SENSOR:
		printf("Geomagnetic [%lld] [%6.6f] [%6.6f] [%6.6f]\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
		break;
	case LIGHT_SENSOR:
		printf("Light [%lld] [%6.6f]\n", data->timestamp, data->values[0]);
		break;
	case TEMPERATURE_SENSOR :
		printf("Temperature [%lld] [%6.6f]\n", data->timestamp, data->values[0]);
		break;
	case PROXIMITY_SENSOR:
		printf("Proximity [%lld] [%6.6f]\n", data->timestamp, data->values[0]);
		break;
	case ULTRAVIOLET_SENSOR:
		printf("Ultraviolet [%lld] [%6.6f]\n", data->timestamp, data->values[0]);
		break;
	case BIO_LED_RED_SENSOR:
		printf("Bio_LED_Red [%lld] [%6.6f]\n", data->timestamp, data->values[0]);
		break;
	case ORIENTATION_SENSOR :
		printf("Orientation [%lld] [%6.6f] [%6.6f] [%6.6f]\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
		break;
	case TILT_SENSOR :
		printf("Tilt [%lld] [%6.6f] [%6.6f]\n", data->timestamp, data->values[0], data->values[1]);
		break;
	case GRAVITY_SENSOR:
		printf("Gravity [%lld] [%6.6f] [%6.6f] [%6.6f]\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
		break;
	case LINEAR_ACCEL_SENSOR:
		printf("Linear acceleration [%lld] [%6.6f] [%6.6f] [%6.6f]\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
		break;
	case ROTATION_VECTOR_SENSOR:
		printf("Rotation vector [%lld] [%6.6f] [%6.6f] [%6.6f] [%6.6f]\n", data->timestamp, data->values[0], data->values[1], data->values[2], data->values[3]);
		break;
	case GEOMAGNETIC_RV_SENSOR:
		printf("Geomagnetic RV [%lld] [%6.6f] [%6.6f] [%6.6f] [%6.6f]\n", data->timestamp, data->values[0], data->values[1], data->values[2], data->values[3]);
		break;
	case GYROSCOPE_RV_SENSOR:
		printf("Gaming RV [%lld] [%6.6f] [%6.6f] [%6.6f] [%6.6f]\n", data->timestamp, data->values[0], data->values[1], data->values[2], data->values[3]);
		break;
	case GYROSCOPE_UNCAL_SENSOR:
		printf("Gyroscope Uncal [%lld] [%6.6f] [%6.6f] [%6.6f] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2], data->values[3], data->values[4], data->values[5]);
		break;
	case GESTURE_FACE_DOWN_SENSOR:
		printf("Face Down [%lld] [%6.6f] \n", data->timestamp, data->values[0]);
		break;

	default:
		return;
	}
}

void *check_sensor(void *arg)
{
	struct pthread_arguments * argu = (struct pthread_arguments *) arg;

	GMainLoop *mainloop;
	int handle;
	bool result, start_handle, stop_handle;

	mainloop = g_main_loop_new(NULL, FALSE);

	sensor_t sensor = sensord_get_sensor(argu -> sensor_type);
	handle = sensord_connect(sensor);

	result = sensord_register_event(handle, argu->event, argu->interval, 0, callback, NULL);

	if (!result) {
		printf("Can't register sensor\n");
		return NULL;
	}

	start_handle = sensord_start(handle, 0);

	if (!start_handle) {
		printf("Error\n\n\n\n");
		sensord_unregister_event(handle, argu->event);
		sensord_disconnect(handle);
		return NULL;
	}

	g_main_loop_run(mainloop);
	g_main_loop_unref(mainloop);

	result = sensord_unregister_event(handle, argu->event);

	if (!result) {
		printf("Error\n\n");
		return NULL;
	}

	stop_handle = sensord_stop(handle);

	if (!stop_handle) {
		printf("Error\n\n");
		return NULL;
	}

	sensord_disconnect(handle);
	return NULL;
}

int polling_sensor(sensor_type_t sensor_type, unsigned int event)
{
	bool result;
	int handle;

	printf("Polling based\n");
	sensor_t sensor;
	sensor = sensord_get_sensor(sensor_type);
	handle = sensord_connect(sensor);
	result = sensord_start(handle, 1);

	if (!result) {
		printf("Can't start the sensor\n");
		printf("Error\n\n\n\n");
		return -1;
	}

	sensor_data_t data;

	while (1) {
		result = sensord_get_data(handle, event, &data);
		printpollinglogs(sensor_type, data);
		usleep(100000);
	}

	result = sensord_disconnect(handle);

	if (!result) {
		printf("Can't disconnect sensor\n");
		printf("Error\n\n\n\n");
		return -1;
	}

	return 0;
}
