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

#include "sensor_manager.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "util.h"
#include "macro.h"

struct sensor_info {
	sensor_type_t type;
	char name[NAME_MAX_TEST];
};

static struct sensor_info sensor_infos[] = {
	{ALL_SENSOR,					"all"},

	// General Sensors
	{ACCELEROMETER_SENSOR,			"accelerometer"},
	{GRAVITY_SENSOR,				"gravity"},
	{LINEAR_ACCEL_SENSOR,			"linear_accel"},
	{GEOMAGNETIC_SENSOR,			"magnetic"},
	{ROTATION_VECTOR_SENSOR,		"rotation_vector"},
	{ORIENTATION_SENSOR,			"orientation"},
	{GYROSCOPE_SENSOR,				"gyroscope"},
	{LIGHT_SENSOR,					"light"},
	{PROXIMITY_SENSOR,				"proximity"},
	{PRESSURE_SENSOR,				"pressure"},
	{ULTRAVIOLET_SENSOR,			"uv"},
	{TEMPERATURE_SENSOR,			"temperature"},
	{HUMIDITY_SENSOR,				"humidity"},
	{HRM_SENSOR,					"hrm"},
	{HRM_RAW_SENSOR,				"hrm_raw"},
	{HRM_LED_GREEN_SENSOR,			"hrm_led_green"},
	{HRM_LED_IR_SENSOR,				"hrm_led_ir"},
	{HRM_LED_RED_SENSOR,			"hrm_led_red"},
	{GYROSCOPE_UNCAL_SENSOR,		"gyro_uncal"},
	{GEOMAGNETIC_UNCAL_SENSOR,		"mag_uncal"},
	{GYROSCOPE_RV_SENSOR,			"gyro_rv"},
	{GEOMAGNETIC_RV_SENSOR,			"mag_rv"},

	{HUMAN_PEDOMETER_SENSOR,		"pedo"},
	{HUMAN_SLEEP_MONITOR_SENSOR,	"sleep_monitor"},

	{AUTO_ROTATION_SENSOR,			"rotation"},
	//{AUTO_BRIGHTENESS_SENSOR,		"auto_brighteness"},
	{MOTION_SENSOR,					"motion"},
	{CONTEXT_SENSOR,				"context"},

	{GESTURE_MOVEMENT_SENSOR,		"movement"},
	{GESTURE_WRIST_UP_SENSOR,		"wristup"},
	{GESTURE_WRIST_DOWN_SENSOR,		"wristdown"},
	{GESTURE_MOVEMENT_STATE_SENSOR,	"movement_state"},

	{WEAR_STATUS_SENSOR,			"wear_status"},
	{WEAR_ON_MONITOR_SENSOR,		"wear_on"},
	{GPS_BATCH_SENSOR,				"gps"},
	{ACTIVITY_TRACKER_SENSOR,		"activity"},
	{SLEEP_DETECTOR_SENSOR,			"sleep_detector"},
};

sensor_manager::~sensor_manager()
{
}

bool sensor_manager::run(int argc, char *argv[])
{
	return true;
}

void sensor_manager::stop(void)
{
}

sensor_type_t sensor_manager::get_sensor_type(const char *name)
{
	int index;
	int count;

	if (util::is_hex(name))
		return (sensor_type_t) (strtol(name, NULL, 16));

	if (util::is_number(name))
		return (sensor_type_t) (atoi(name));

	count = ARRAY_SIZE(sensor_infos);

	for (index = 0; index < count; ++index) {
		if (!strcmp(sensor_infos[index].name, name))
			break;
	}

	if (index == count) {
		_E("Invaild sensor name\n");
		usage_sensors();
		return UNKNOWN_SENSOR;
	}
	return sensor_infos[index].type;
}

void sensor_manager::usage_sensors(void)
{
	_N("The sensor types are:\n");
	int sensor_count = ARRAY_SIZE(sensor_infos);

	for (int i = 0; i < sensor_count; ++i)
		_N("%3d: %s(%#x)\n", i, sensor_infos[i].name, sensor_infos[i].type);
	_N("\n");
}
