/*
 * sensorctl
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#include <string.h>
#include <macro.h>
#include <sensorctl_log.h>
#include "sensor_manager.h"

#define NAME_MAX_TEST 32

struct sensor_info {
	sensor_type_t type;
	char name[NAME_MAX_TEST];
};

static struct sensor_info sensor_infos[] = {
	{ALL_SENSOR,				"all"},

	// General Sensors
	{ACCELEROMETER_SENSOR,		"accelerometer"},
	{GEOMAGNETIC_SENSOR,		"magnetic"},
	{LIGHT_SENSOR,				"light"},
	{PROXIMITY_SENSOR,			"proximity"},
	{GYROSCOPE_SENSOR,			"gyroscope"},
	{PRESSURE_SENSOR,			"pressure"},
	{BIO_SENSOR,				"bio"},
	{BIO_HRM_SENSOR,			"hrm"},
	{AUTO_ROTATION_SENSOR,		"auto_rotation"},
	{GRAVITY_SENSOR,			"gravity"},
	{LINEAR_ACCEL_SENSOR,		"linear_accel"},
	{ROTATION_VECTOR_SENSOR,	"rotation_vector"},
	{ORIENTATION_SENSOR,		"orientation"},
	{TEMPERATURE_SENSOR,		"temperature"},
	{HUMIDITY_SENSOR,			"humidity"},
	{ULTRAVIOLET_SENSOR,		"ultraviolet"},
	{BIO_LED_GREEN_SENSOR,		"hrm_led_green"},
	{BIO_LED_IR_SENSOR,			"hrm_led_ir"},
	{BIO_LED_RED_SENSOR,		"hrm_led_red"},
	{GYROSCOPE_UNCAL_SENSOR,	"gyro_uncal"},
	{GEOMAGNETIC_UNCAL_SENSOR,	"mag_uncal"},
	{GYROSCOPE_RV_SENSOR,		"gyro_rv"},
	{GEOMAGNETIC_RV_SENSOR,		"mag_rv"},
	/* If WRIST_UP_SENSOR is created, it has to be changed to WRIST_UP_SENSOR */
	{MOTION_SENSOR,				"motion"},
	{CONTEXT_SENSOR,			"context"},
	{EXERCISE_SENSOR,			"exercise"},
	{GESTURE_WRIST_UP_SENSOR,	"wristup"},
};

bool sensor_manager::process(int argc, char *argv[])
{
	return false;
}

void sensor_manager::usage_sensors(void)
{
	PRINT("The sensor types are:\n");
	int sensor_count = ARRAY_SIZE(sensor_infos);

	for (int i = 0; i < sensor_count; ++i)
		PRINT("  %d: %s(%d)\n", i, sensor_infos[i].name, sensor_infos[i].type);
	PRINT("\n");
}

sensor_type_t sensor_manager::get_sensor_type(char *name)
{
	int index;
	int sensor_count = ARRAY_SIZE(sensor_infos);

	for (index = 0; index < sensor_count; ++index) {
		if (!strcmp(sensor_infos[index].name, name))
			break;
	}

	if (index == sensor_count) {
		_E("ERROR: sensor name is wrong\n");
		usage_sensors();
		return UNKNOWN_SENSOR;
	}
	return sensor_infos[index].type;
}

const char *sensor_manager::get_sensor_name(sensor_type_t type)
{
	int index;
	int sensor_count = ARRAY_SIZE(sensor_infos);

	for (index = 0; index < sensor_count; ++index) {
		if (sensor_infos[index].type == type)
			break;
	}

	if (index == sensor_count) {
		_E("ERROR: sensor name is wrong\n");
		usage_sensors();
		return "UNKNOWN SENSOR";
	}
	return sensor_infos[index].name;
}
