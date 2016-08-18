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

#include "../../orientation_sensor.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#define ORIENTATION_DATA_PATH "../../../design/data/100ms/orientation/roll_pitch_yaw/"
#define ORIENTATION_DATA_SIZE 1095
int pitch_phase_compensation = -1;
int roll_phase_compensation = -1;
int azimuth_phase_compensation = -1;

int main()
{
	int data_available = ORIENTATION_DATA_SIZE;
	ifstream accel_in, gyro_in, mag_in;
	ofstream orien_file;
	string line_accel, line_gyro, line_magnetic;
	float sdata[3];
	unsigned long long time_stamp;
	euler_angles<float> orientation;
	rotation_matrix<float> orientation_mat;
	quaternion<float> orientation_9axis_quat;
	quaternion<float> orientation_geomagnetic_quat;
	quaternion<float> orientation_gaming_quat;
	orientation_sensor orien_sensor;

	accel_in.open(((string)ORIENTATION_DATA_PATH + (string)"accel.txt").c_str());
	gyro_in.open(((string)ORIENTATION_DATA_PATH + (string)"gyro.txt").c_str());
	mag_in.open(((string)ORIENTATION_DATA_PATH + (string)"magnetic.txt").c_str());

	orien_file.open(((string)"orientation.txt").c_str());

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

		orien_sensor.get_device_orientation(&accel_data, &gyro_data, &magnetic_data);

		orientation = orien_sensor.orien_filter.m_orientation;

		cout << "Gyro Bias in radians\t" << orien_sensor.orien_filter.m_gyro_bias;

		orientation = rad2deg(orientation);

		orientation.m_ang.m_vec[0] *= pitch_phase_compensation;
		orientation.m_ang.m_vec[1] *= roll_phase_compensation;
		orientation.m_ang.m_vec[2] *= azimuth_phase_compensation;

		if (orientation.m_ang.m_vec[2] < 0)
			orientation.m_ang.m_vec[2] += 360;

		orien_file << orientation.m_ang;

		orientation = orien_sensor.orien_filter.m_orientation;

		cout << "Orientation angles\t" << orientation.m_ang << "\n\n";

		cout << "Orientation matrix\t" << orien_sensor.orien_filter.m_rot_matrix.m_rot_mat << "\n\n";

		cout << "Orientation 9-axis quaternion\t" << orien_sensor.orien_filter.m_quat_9axis.m_quat << "\n\n";

		cout << "Orientation geomagnetic quaternion\t" << orien_sensor.orien_filter.m_quat_aid.m_quat << "\n\n";

		cout << "Orientation gaming quaternion\t" << orien_sensor.orien_filter.m_quat_gaming_rv.m_quat << "\n\n";
	}

	accel_in.close();
	gyro_in.close();
	mag_in.close();
	orien_file.close();

	return 0;
}
