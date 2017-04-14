/*
 * sensord
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

#include "sensor_utils.h"

#include <glib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stddef.h>
#include <map>

#include <sensor_types.h>

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

/* TODO: move and define string type to sensor_type.h */
static std::map<sensor_type_t, const char *> types = {
	{UNKNOWN_SENSOR,                 "http://tizen.org/sensor/unknown"},
	{ALL_SENSOR,                     "http://tizen.org/sensor/all"},
	{ACCELEROMETER_SENSOR,           "http://tizen.org/sensor/accelerometer"},
	{GRAVITY_SENSOR,                 "http://tizen.org/sensor/gravity"},
	{LINEAR_ACCEL_SENSOR,            "http://tizen.org/sensor/linear_accel"},
	{GEOMAGNETIC_SENSOR,             "http://tizen.org/sensor/geomagnetic"},
	{ROTATION_VECTOR_SENSOR,         "http://tizen.org/sensor/rotation_vector"},
	{ORIENTATION_SENSOR,             "http://tizen.org/sensor/orientation"},
	{GYROSCOPE_SENSOR,               "http://tizen.org/sensor/gyroscope"},
	{LIGHT_SENSOR,                   "http://tizen.org/sensor/light"},
	{PROXIMITY_SENSOR,               "http://tizen.org/sensor/proximity"},
	{PRESSURE_SENSOR,                "http://tizen.org/sensor/pressure"},
	{ULTRAVIOLET_SENSOR,             "http://tizen.org/sensor/ultraviolet"},
	{TEMPERATURE_SENSOR,             "http://tizen.org/sensor/temperature"},
	{HUMIDITY_SENSOR,                "http://tizen.org/sensor/humidity"},
	{HRM_SENSOR,                     "http://tizen.org/sensor/heart_rate_monitor"},
	{HRM_LED_GREEN_SENSOR,           "http://tizen.org/sensor/hrm_led_green"},
	{HRM_LED_IR_SENSOR,              "http://tizen.org/sensor/hrm_led_ir"},
	{HRM_LED_RED_SENSOR,             "http://tizen.org/sensor/hrm_led_red"},
	{GYROSCOPE_UNCAL_SENSOR,         "http://tizen.org/sensor/gyro_uncalibrated"},
	{GEOMAGNETIC_UNCAL_SENSOR,       "http://tizen.org/sensor/mag_uncalibrated"},
	{GYROSCOPE_RV_SENSOR,            "http://tizen.org/sensor/gyro_rotation_vector"},
	{GEOMAGNETIC_RV_SENSOR,          "http://tizen.org/sensor/mag_rotation_vector"},

	{HUMAN_PEDOMETER_SENSOR,         "http://tizen.org/sensor/human_pedometer"},
	{HUMAN_SLEEP_MONITOR_SENSOR,     "http://tizen.org/sensor/human_sleep_monitor"},
	{HUMAN_SLEEP_DETECTOR_SENSOR,    "http://tizen.org/sensor/human_sleep_detector"},
	{HUMAN_STRESS_MONITOR_SENSOR,    "http://tizen.org/sensor/human_stress_monitor"},

	{EXERCISE_WALKING_SENSOR,        "http://tizen.org/sensor/exercise.walking"},
	{EXERCISE_RUNNING_SENSOR,        "http://tizen.org/sensor/exercise.running"},
	{EXERCISE_HIKING_SENSOR,         "http://tizen.org/sensor/exercise.hiking"},
	{EXERCISE_CYCLING_SENSOR,        "http://tizen.org/sensor/exercise.cycling"},
	{EXERCISE_ELLIPTICAL_SENSOR,     "http://tizen.org/sensor/exercise.elliptical"},
	{EXERCISE_INDOOR_CYCLING_SENSOR, "http://tizen.org/sensor/exercise.indoor_cycling"},
	{EXERCISE_ROWING_SENSOR,         "http://tizen.org/sensor/exercise.rowing"},
	{EXERCISE_STEPPER_SENSOR,        "http://tizen.org/sensor/exercise.stepper"},

	{EXTERNAL_EXERCISE_SENSOR,       "http://tizen.org/sensor/external_exercise"},

	{FUSION_SENSOR,                  "http://tizen.org/sensor/fusion"},
	{AUTO_ROTATION_SENSOR,           "http://tizen.org/sensor/auto_rotation"},
	{AUTO_BRIGHTNESS_SENSOR,         "http://tizen.org/sensor/auto_brightness"},

	{GESTURE_MOVEMENT_SENSOR,        "http://tizen.org/sensor/gesture_movement"},
	{GESTURE_WRIST_UP_SENSOR,        "http://tizen.org/sensor/gesture_wrist_up"},
	{GESTURE_WRIST_DOWN_SENSOR,      "http://tizen.org/sensor/gesture_wrist_down"},
	{GESTURE_MOVEMENT_STATE_SENSOR,  "http://tizen.org/sensor/gesture_movement_state"},
	{GESTURE_FACE_DOWN_SENSOR,       "http://tizen.org/sensor/gesture_face_down"},

	{ACTIVITY_TRACKER_SENSOR,        "http://tizen.org/sensor/activity_tracker"},
	{ACTIVITY_LEVEL_MONITOR_SENSOR,  "http://tizen.org/sensor/activity_level_monitor"},
	{GPS_BATCH_SENSOR,               "http://tizen.org/sensor/gps_batch"},

	{HRM_CTRL_SENSOR,                "http://tizen.org/sensor/hrm_ctrl"},

	{WEAR_STATUS_SENSOR,             "http://tizen.org/sensor/wear_status"},
	{WEAR_ON_MONITOR_SENSOR,         "http://tizen.org/sensor/wear_on_monitor"},
	{NO_MOVE_DETECTOR_SENSOR,        "http://tizen.org/sensor/no_move_detector"},
	{RESTING_HR_SENSOR,              "http://tizen.org/sensor/resting_hr"},
	{STEP_LEVEL_MONITOR_SENSOR,      "http://tizen.org/sensor/step_level_monitor"},
	{EXERCISE_STANDALONE_SENSOR,     "http://tizen.org/sensor/exercise_standalone"},
	{EXERCISE_HR_SENSOR,             "http://tizen.org/sensor/exercise_hr"},
	{WORKOUT_SENSOR,                 "http://tizen.org/sensor/workout"},
	{CYCLE_MONITOR_SENSOR,           "http://tizen.org/sensor/cycle_monitor"},
	{STAIR_TRACKER_SENSOR,           "http://tizen.org/sensor/stair_tracker"},
	{PRESSURE_INDICATOR_SENSOR,      "http://tizen.org/sensor/pressure_indicator"},
	{PRESSURE_ALERT_SENSOR,          "http://tizen.org/sensor/pressure_alert"},
	{HR_CALORIE_SENSOR,              "http://tizen.org/sensor/hr_calorie"},

	{CONTEXT_SENSOR,                 "http://tizen.org/sensor/context"},
	{MOTION_SENSOR,                  "http://tizen.org/sensor/motion"},
	{PIR_SENSOR,                     "http://tizen.org/sensor/pir"},
	{PIR_LONG_SENSOR,                "http://tizen.org/sensor/pir_long"},
	{DUST_SENSOR,                    "http://tizen.org/sensor/dust"},
	{THERMOMETER_SENSOR,             "http://tizen.org/sensor/thermometer"},
	{PEDOMETER_SENSOR,               "http://tizen.org/sensor/pedometer"},
	{FLAT_SENSOR,                    "http://tizen.org/sensor/flat"},
	{HRM_RAW_SENSOR,                 "http://tizen.org/sensor/hrm_raw"},
	{TILT_SENSOR,                    "http://tizen.org/sensor/tilt"},
	{RV_RAW_SENSOR,                  "http://tizen.org/sensor/rv_raw"},
	{GSR_SENSOR,                     "http://tizen.org/sensor/gsr"},
	{SIMSENSE_SENSOR,                "http://tizen.org/sensor/simsense"},
	{PPG_SENSOR,                     "http://tizen.org/sensor/ppg"},
};

const char *sensor::utils::get_uri(sensor_type_t type)
{
	auto it = types.find(type);
	if (it == types.end())
		return "Unknown Type";
	return it->second;
}

unsigned long long sensor::utils::get_timestamp(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return ((unsigned long long)(t.tv_sec)*1000000000LL + t.tv_nsec) / 1000;
}

unsigned long long sensor::utils::get_timestamp(timeval *t)
{
	if (!t)
		return 0;

	return ((unsigned long long)(t->tv_sec)*1000000LL +t->tv_usec);
}

#ifdef _DEBUG
bool sensor::utils::get_proc_name(pid_t pid, char *process_name)
{
	FILE *fp;
	char buf[NAME_MAX];
	char filename[PATH_MAX];

	sprintf(filename, "/proc/%d/stat", pid);
	fp = fopen(filename, "r");

	if (fp == NULL)
		return false;

	if (fscanf(fp, "%*s (%[^)]", buf) < 1) {
		fclose(fp);
		return false;
	}

	strncpy(process_name, buf, NAME_MAX-1);
	process_name[NAME_MAX-1] = '\0';
	fclose(fp);

	return true;
}
#else
bool sensor::utils::get_proc_name(pid_t pid, char *process_name)
{
	char buf[NAME_MAX];

	if (snprintf(buf, sizeof(buf), "%d process", pid) < 1) {
		return false;
	}

	strncpy(process_name, buf, NAME_MAX-1);
	process_name[NAME_MAX-1] = '\0';

	return true;
}
#endif

const char* sensor::utils::get_client_name(void)
{
	const int pid_string_size = 10;
	static pid_t pid = -1;
	static char client_name[NAME_MAX + pid_string_size];

	char proc_name[NAME_MAX];

	if (pid == -1)
	{
		pid = getpid();
		get_proc_name(pid, proc_name);
		snprintf(client_name, sizeof(client_name), "%s(%d)", proc_name, pid);
	}

	return client_name;
}

std::vector<std::string> sensor::utils::tokenize(const std::string &in, const char *delim)
{
	std::vector<std::string> tokens;
	char *input = g_strdup(in.c_str());

	char *save = NULL;
	char *token = strtok_r(input, delim, &save);

	while (token != NULL) {
		tokens.push_back(token);
		token = strtok_r(NULL, delim, &save);
	}

	g_free(input);
	return tokens;
}
