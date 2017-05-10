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

#include <sensor_log.h>
#include <sensor_types.h>
#include <sensor_types_private.h>

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

/* TODO: move and define string type to sensor_type.h */
static std::map<sensor_type_t, const char *> types = {
	{UNKNOWN_SENSOR,                 "http://tizen.org/sensor/general/unknown"},
	{ALL_SENSOR,                     "http://tizen.org/sensor/general/all"},
	{ACCELEROMETER_SENSOR,           "http://tizen.org/sensor/general/accelerometer"},
	{GRAVITY_SENSOR,                 "http://tizen.org/sensor/general/gravity"},
	{LINEAR_ACCEL_SENSOR,            "http://tizen.org/sensor/general/linear_acceleration"},
	{GEOMAGNETIC_SENSOR,             "http://tizen.org/sensor/general/magnetic"},
	{ROTATION_VECTOR_SENSOR,         "http://tizen.org/sensor/general/rotation_vector"},
	{ORIENTATION_SENSOR,             "http://tizen.org/sensor/general/orientation"},
	{GYROSCOPE_SENSOR,               "http://tizen.org/sensor/general/gyroscope"},
	{LIGHT_SENSOR,                   "http://tizen.org/sensor/general/light"},
	{PROXIMITY_SENSOR,               "http://tizen.org/sensor/general/proximity"},
	{PRESSURE_SENSOR,                "http://tizen.org/sensor/general/pressure"},
	{ULTRAVIOLET_SENSOR,             "http://tizen.org/sensor/general/ultraviolet"},
	{TEMPERATURE_SENSOR,             "http://tizen.org/sensor/general/temperature"},
	{HUMIDITY_SENSOR,                "http://tizen.org/sensor/general/humidity"},
	{HRM_SENSOR,                     "http://tizen.org/sensor/healthinfo/heart_rate_monitor"},
	{HRM_LED_GREEN_SENSOR,           "http://tizen.org/sensor/healthinfo/heart_rate_monitor.led_green"},
	{HRM_LED_IR_SENSOR,              "http://tizen.org/sensor/healthinfo/heart_rate_monitor.led_ir"},
	{HRM_LED_RED_SENSOR,             "http://tizen.org/sensor/healthinfo/heart_rate_monitor.led_red"},
	{GYROSCOPE_UNCAL_SENSOR,         "http://tizen.org/sensor/general/gyroscope.uncalibrated"},
	{GEOMAGNETIC_UNCAL_SENSOR,       "http://tizen.org/sensor/general/geomagnetic.uncalibrated"},
	{GYROSCOPE_RV_SENSOR,            "http://tizen.org/sensor/general/gyroscope_rotation_vector"},
	{GEOMAGNETIC_RV_SENSOR,          "http://tizen.org/sensor/general/geomagnetic_rotation_vector"},

	{HUMAN_PEDOMETER_SENSOR,         "http://tizen.org/sensor/healthinfo/human_pedometer"},
	{HUMAN_SLEEP_MONITOR_SENSOR,     "http://tizen.org/sensor/healthinfo/human_sleep_monitor"},
	{HUMAN_SLEEP_DETECTOR_SENSOR,    "http://tizen.org/sensor/healthinfo/human_sleep_detector"},
	{HUMAN_STRESS_MONITOR_SENSOR,    "http://tizen.org/sensor/healthinfo/human_stress_monitor"},

	{EXERCISE_WALKING_SENSOR,        "http://tizen.org/sensor/general/exercise.walking"},
	{EXERCISE_RUNNING_SENSOR,        "http://tizen.org/sensor/general/exercise.running"},
	{EXERCISE_HIKING_SENSOR,         "http://tizen.org/sensor/general/exercise.hiking"},
	{EXERCISE_CYCLING_SENSOR,        "http://tizen.org/sensor/general/exercise.cycling"},
	{EXERCISE_ELLIPTICAL_SENSOR,     "http://tizen.org/sensor/general/exercise.elliptical"},
	{EXERCISE_INDOOR_CYCLING_SENSOR, "http://tizen.org/sensor/general/exercise.indoor_cycling"},
	{EXERCISE_ROWING_SENSOR,         "http://tizen.org/sensor/general/exercise.rowing"},
	{EXERCISE_STEPPER_SENSOR,        "http://tizen.org/sensor/general/exercise.stepper"},

	{EXTERNAL_EXERCISE_SENSOR,       "http://tizen.org/sensor/general/external_exercise"},

	{FUSION_SENSOR,                  "http://tizen.org/sensor/general/fusion"},
	{AUTO_ROTATION_SENSOR,           "http://tizen.org/sensor/general/auto_rotation"},
	{AUTO_BRIGHTNESS_SENSOR,         "http://tizen.org/sensor/general/auto_brightness"},

	{GESTURE_MOVEMENT_SENSOR,        "http://tizen.org/sensor/general/gesture_movement"},
	{GESTURE_WRIST_UP_SENSOR,        "http://tizen.org/sensor/general/gesture_wrist_up"},
	{GESTURE_WRIST_DOWN_SENSOR,      "http://tizen.org/sensor/general/gesture_wrist_down"},
	{GESTURE_MOVEMENT_STATE_SENSOR,  "http://tizen.org/sensor/general/gesture_movement_state"},
	{GESTURE_FACE_DOWN_SENSOR,       "http://tizen.org/sensor/general/gesture_face_down"},

	{ACTIVITY_TRACKER_SENSOR,        "http://tizen.org/sensor/general/activity_tracker"},
	{ACTIVITY_LEVEL_MONITOR_SENSOR,  "http://tizen.org/sensor/general/activity_level_monitor"},
	{GPS_BATCH_SENSOR,               "http://tizen.org/sensor/general/gps_batch"},

	{HRM_CTRL_SENSOR,                "http://tizen.org/sensor/general/hrm_ctrl"},

	{WEAR_STATUS_SENSOR,             "http://tizen.org/sensor/general/wear_status"},
	{WEAR_ON_MONITOR_SENSOR,         "http://tizen.org/sensor/general/wear_on_monitor"},
	{NO_MOVE_DETECTOR_SENSOR,        "http://tizen.org/sensor/general/no_move_detector"},
	{RESTING_HR_SENSOR,              "http://tizen.org/sensor/general/resting_hr"},
	{STEP_LEVEL_MONITOR_SENSOR,      "http://tizen.org/sensor/general/step_level_monitor"},
	{EXERCISE_STANDALONE_SENSOR,     "http://tizen.org/sensor/general/exercise_standalone"},
	{EXERCISE_HR_SENSOR,             "http://tizen.org/sensor/healthinfo/general/exercise_hr"},
	{WORKOUT_SENSOR,                 "http://tizen.org/sensor/general/workout"},
	{CYCLE_MONITOR_SENSOR,           "http://tizen.org/sensor/general/cycle_monitor"},
	{STAIR_TRACKER_SENSOR,           "http://tizen.org/sensor/general/stair_tracker"},
	{PRESSURE_INDICATOR_SENSOR,      "http://tizen.org/sensor/general/pressure_indicator"},
	{PRESSURE_ALERT_SENSOR,          "http://tizen.org/sensor/general/pressure_alert"},
	{HR_CALORIE_SENSOR,              "http://tizen.org/sensor/general/hr_calorie"},

	{CONTEXT_SENSOR,                 "http://tizen.org/sensor/general/context"},
	{MOTION_SENSOR,                  "http://tizen.org/sensor/general/motion"},
	{PIR_SENSOR,                     "http://tizen.org/sensor/general/pir"},
	{PIR_LONG_SENSOR,                "http://tizen.org/sensor/general/pir_long"},
	{DUST_SENSOR,                    "http://tizen.org/sensor/general/dust"},
	{THERMOMETER_SENSOR,             "http://tizen.org/sensor/general/thermometer"},
	{PEDOMETER_SENSOR,               "http://tizen.org/sensor/healthinfo/general/pedometer"},
	{FLAT_SENSOR,                    "http://tizen.org/sensor/general/flat"},
	{HRM_RAW_SENSOR,                 "http://tizen.org/sensor/healthinfo/general/hrm_raw"},
	{TILT_SENSOR,                    "http://tizen.org/sensor/general/tilt"},
	{RV_RAW_SENSOR,                  "http://tizen.org/sensor/general/rv_raw"},
	{GSR_SENSOR,                     "http://tizen.org/sensor/healthinfo/general/gsr"},
	{SIMSENSE_SENSOR,                "http://tizen.org/sensor/general/simsense"},
	{PPG_SENSOR,                     "http://tizen.org/sensor/healthinfo/general/ppg"},
};

const char *sensor::utils::get_uri(sensor_type_t type)
{
	auto it = types.find(type);
	if (it == types.end())
		return "Unknown Type";
	return it->second;
}

const char *sensor::utils::get_privilege(std::string uri)
{
	std::size_t start = 0;
	std::size_t end = uri.length();
	std::size_t size = uri.size();

	for (int i = 0; i < URI_PRIV_INDEX; ++i) {
		retv_if(start >= uri.length(), "");
		start = uri.find(URI_DELIMITER, start + 1);
		retv_if(start == std::string::npos, "");
	}

	end = uri.find(URI_DELIMITER, start + 1);
	retv_if(end == std::string::npos, "");

	size = end - (start + 1);

	if (uri.substr(start + 1, size) == PRIVILEGE_HEALTHINFO_STR)
		return PRIVILEGE_HEALTHINFO_URI;

	return "";
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
