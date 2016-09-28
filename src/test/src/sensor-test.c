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
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <sensor_internal.h>
#include <stdbool.h>
#include <sensor_common.h>
#include <unistd.h>
#include <string.h>

#include "check-sensor.h"

void usage()
{
	printf("Usage : ./sensor-test <Sensor_type> -p(optional) <event>(optional) <interval>(optional)\n\n");

	printf("Sensor_type: ");
	printf("[accelerometer] ");
	printf("[auto_rotation]\n");
	printf("[gyroscope] ");
	printf("[pressure] ");
	printf("[temperature] ");
	printf("[geomagnetic] ");
	printf("[orientation] ");
	printf("[tilt] ");
	printf("[gravity] ");
	printf("[linear_accel] ");
	printf("[rotation_vector] ");
	printf("[geomagnetic_rv] ");
	printf("[gaming_rv] ");
	printf("[ultraviolet] ");
	printf("[bio_led_red] ");
	printf("[light]\n");
	printf("[gyroscope_uncal]");
	printf("[face_down]");
	printf("event:");
	printf("[RAW_DATA_EVENT]\n");
	printf("-p: [polling]\n");
	printf("Sensor_type: ");
	printf("[proximity]\n");
	printf("event:");
	printf("[CHANGE_STATE_EVENT]\n");
	printf("-p: [polling]\n");
	printf("interval:\n");
	printf("The time interval should be entered based on the sampling frequency supported by accelerometer driver on the device in ms.If no value for sensor is entered default value by the driver will be used.\n");
}

int main(int argc, char **argv)
{
	int interval;
	unsigned int event;
	sensor_type_t sensor_type;
	bool is_polling;

	char *end1;

	if (argc < 2 || argc > 5) {
		printf("Wrong number of arguments\n");
		usage();
		return -1;
	}

	if (strcmp(argv[1], "accelerometer") == 0) {
		 sensor_type = ACCELEROMETER_SENSOR;
		 event = ACCELEROMETER_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "auto_rotation") == 0) {
		 sensor_type = AUTO_ROTATION_SENSOR;
		 event = AUTO_ROTATION_CHANGE_STATE_EVENT;
	}
	else if (strcmp(argv[1], "gyroscope") == 0) {
		 sensor_type = GYROSCOPE_SENSOR;
		 event = GYROSCOPE_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "pressure") == 0) {
		 sensor_type = PRESSURE_SENSOR;
		 event = PRESSURE_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "temperature") == 0) {
		 sensor_type = TEMPERATURE_SENSOR;
		 event = TEMPERATURE_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "geomagnetic") == 0) {
		 sensor_type = GEOMAGNETIC_SENSOR;
		 event = GEOMAGNETIC_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "orientation") == 0) {
		 sensor_type = ORIENTATION_SENSOR;
		 event = ORIENTATION_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "tilt") == 0) {
		 sensor_type = TILT_SENSOR;
		 event = TILT_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "gravity") == 0) {
		 sensor_type = GRAVITY_SENSOR;
		 event = GRAVITY_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "linear_accel") == 0) {
		 sensor_type = LINEAR_ACCEL_SENSOR;
		 event = LINEAR_ACCEL_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "rotation_vector") == 0) {
		 sensor_type = ROTATION_VECTOR_SENSOR;
		 event = ROTATION_VECTOR_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "geomagnetic_rv") == 0) {
		 sensor_type = GEOMAGNETIC_RV_SENSOR;
		 event = GEOMAGNETIC_RV_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "gaming_rv") == 0) {
		 sensor_type = GYROSCOPE_RV_SENSOR;
		 event = GAMING_RV_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "light") == 0) {
		 sensor_type = LIGHT_SENSOR;
		 event = LIGHT_LUX_DATA_EVENT;
	}
	else if (strcmp(argv[1], "proximity") == 0) {
		 sensor_type = PROXIMITY_SENSOR;
		 event = PROXIMITY_CHANGE_STATE_EVENT;
	}
	else if (strcmp(argv[1], "ultraviolet") == 0) {
		 sensor_type = ULTRAVIOLET_SENSOR;
		 event = ULTRAVIOLET_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "gyroscope_uncal") == 0) {
		 sensor_type = GYROSCOPE_UNCAL_SENSOR;
		 event = GYROSCOPE_UNCAL_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "bio_led_red") == 0) {
		 sensor_type = BIO_LED_RED_SENSOR;
		 event = BIO_LED_RED_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "face_down") == 0) {
		 sensor_type = GESTURE_FACE_DOWN_SENSOR;
		 event = FACE_DOWN_RAW_DATA_EVENT;
	}
	else {
		 usage();
		 return -1;
	}

	interval = DEFAULT_EVENT_INTERVAL;

	is_polling = FALSE;

	if(argc >= 3 && strcmp(argv[2], "-p") == 0) {
		is_polling = TRUE;
	}

	if (is_polling) {
		if (argc == 4) {
			int temp_event = get_event(sensor_type, argv[3]);
			if (temp_event == -1) {
				interval = atoi(argv[3]);
				if (interval == 0){
					usage();
					return -1;
				}
			}
			else {
				event = temp_event;
			}
		}
		else if (argc == 5) {
			event = get_event(sensor_type, argv[3]);
			interval = strtol(argv[4], &end1, 10);

			if (*end1) {
				printf("Conversion error, non-convertible part: %s\n", end1);
				return -1;
			}
		}
		return polling_sensor(sensor_type, event);
	}
	else {
		if (argc == 3) {
			int temp_event = get_event(sensor_type, argv[2]);

			if (temp_event == -1) {
				interval = atoi(argv[2]);
				if (interval == 0){
					usage();
					return -1;
				}
			}
			else {
				event = temp_event;
			}
		}
		else if (argc == 4) {
			event = get_event(sensor_type, argv[2]);
			interval = strtol(argv[3], &end1, 10);

			if (*end1) {
				printf("Conversion error, non-convertible part: %s\n", end1);
				return -1;
			}
		}
		struct pthread_arguments arg;
		arg.sensor_type = sensor_type;
		arg.event = event;
		arg.interval = interval;

		void *result = check_sensor((void*)&arg);

		if (!result)
			return -1;

		return 0;
	}
}
