/*
 * sensord
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
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <sensor_internal.h>
#include <stdbool.h>
#include <sensor_common.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "check-sensor.h"

void usage()
{
	printf("Usage : ./multi-sensor <TIMEOUT> <interval>(optional)\n\n");
	printf("TIMEOUT:\n");
	printf("time for which the parallel sensor test cases should run\n");

	printf("interval:\n");
	printf("The time interval should be entered based on the sampling frequency supported by accelerometer driver on the device in ms.\n");
	printf("If no value for sensor is entered default value by the driver will be used.\n");
	printf("arg[i].sensor_type: ");
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
	printf("[light]\n");
	printf("[gyro_uncal]");
	printf("[face_down]");

}

int main(int argc, char **argv)
{

	int i = 0;
	int interval = DEFAULT_EVENT_INTERVAL;
	int TIMEOUT = 10; //in seconds for which all the sensor tests should run

	if(argc < 2) {
		usage();
		return -1;
	}
	else if(argc == 2){
		TIMEOUT = atoi(argv[1]);
		if (TIMEOUT == 0) {
			usage();
			return -1;
		}
	}
	else {
		TIMEOUT = atoi(argv[1]);
		interval = atoi(argv[2]);
		if (TIMEOUT == 0 || interval == 0) {
			usage();
			return -1;
		}
	}

	int MAX = 6, j = 0, k = 0;
	struct pthread_arguments arg[MAX];
	int t = 0;

	arg[0].sensor_type = ACCELEROMETER_SENSOR;
	arg[0].event = ACCELEROMETER_RAW_DATA_EVENT;
	arg[1].sensor_type = GYROSCOPE_SENSOR;
	arg[1].event = GYROSCOPE_RAW_DATA_EVENT;
	arg[2].sensor_type = GEOMAGNETIC_RV_SENSOR;
	arg[2].event = GEOMAGNETIC_RV_RAW_DATA_EVENT;
	arg[3].sensor_type = PRESSURE_SENSOR;
	arg[3].event = PRESSURE_RAW_DATA_EVENT;
	arg[4].sensor_type = PROXIMITY_SENSOR;
	arg[4].event = PROXIMITY_CHANGE_STATE_EVENT;
	arg[5].sensor_type = LIGHT_SENSOR;
	arg[5].event = LIGHT_LUX_DATA_EVENT;

	for (t = 0; t < MAX; t++)
		arg[t].interval = interval;

	pthread_t thread_id[MAX];

	for (j = 0; j < MAX; j++)
		pthread_create(&thread_id[j], NULL, check_sensor, (void*)&arg[j]);

	sleep(TIMEOUT);
	return 0;
}
