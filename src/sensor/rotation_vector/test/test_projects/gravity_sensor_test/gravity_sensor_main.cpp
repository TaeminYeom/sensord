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

#include "../../gravity_sensor.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#define GRAVITY_DATA_PATH "../../../design/data/100ms/gravity/throw/"
#define GRAVITY_DATA_SIZE 135

int main()
{
	int data_available = GRAVITY_DATA_SIZE;
	ifstream accel_in, gyro_in, mag_in;
	ofstream gravity_file;
	string line_accel, line_gyro, line_magnetic;
	float sdata[3];
	unsigned long long time_stamp;
	sensor_data<float> gravity;
	gravity_sensor grav_sensor;

	accel_in.open(((string)GRAVITY_DATA_PATH + (string)"accel.txt").c_str());
	gyro_in.open(((string)GRAVITY_DATA_PATH + (string)"gyro.txt").c_str());
	mag_in.open(((string)GRAVITY_DATA_PATH + (string)"magnetic.txt").c_str());

	gravity_file.open(((string)"gravity.txt").c_str());

	char *token = NULL;

	while (data_available-- > 0)
	{
		getline(accel_in, line_accel);
		sdata[0] = strtof(line_accel.c_str(), &token);
		sdata[1] = strtof(token, &token);
		sdata[2] = strtof(token, &token);
		time_stamp = strtoull(token, NULL, 10);
		sensor_data<float> accel_data(sdata[0], sdata[1], sdata[2], time_stamp);

		cout << "Accel Data\t" << accel_data.m_data << "\t Time Stamp\t" << accel_data.m_time_stamp << "\n\n";

		getline(gyro_in, line_gyro);
		sdata[0] = strtof(line_gyro.c_str(), &token);
		sdata[1] = strtof(token, &token);
		sdata[2] = strtof(token, &token);
		time_stamp = strtoull(token, NULL, 10);
		sensor_data<float> gyro_data(sdata[0], sdata[1], sdata[2], time_stamp);

		cout << "Gyro Data\t" << gyro_data.m_data << "\t Time Stamp\t" << gyro_data.m_time_stamp << "\n\n";

		getline(mag_in, line_magnetic);
		sdata[0] = strtof(line_magnetic.c_str(), &token);
		sdata[1] = strtof(token, &token);
		sdata[2] = strtof(token, &token);
		time_stamp = strtoull(token, NULL, 10);
		sensor_data<float> magnetic_data(sdata[0], sdata[1], sdata[2], time_stamp);

		cout << "Magnetic Data\t" << magnetic_data.m_data << "\t Time Stamp\t" << magnetic_data.m_time_stamp << "\n\n";

		gravity = grav_sensor.get_gravity(accel_data, gyro_data, magnetic_data);

		gravity_file << gravity.m_data;

		cout << "Gravity Data\t" << gravity.m_data << "\n\n";
	}

	accel_in.close();
	gyro_in.close();
	mag_in.close();
	gravity_file.close();

	return 0;
}
