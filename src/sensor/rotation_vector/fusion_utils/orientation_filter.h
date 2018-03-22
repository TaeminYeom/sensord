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

#ifndef _ORIENTATION_FILTER_H_
#define _ORIENTATION_FILTER_H_

#include "matrix.h"
#include "vector.h"
#include "sensor_data.h"
#include "quaternion.h"
#include "euler_angles.h"
#include "rotation_matrix.h"

#define MOVING_AVERAGE_WINDOW_LENGTH	20

#define M3X3R	3
#define M3X3C	3
#define M6X6R	6
#define M6X6C	6
#define V1x3S	3
#define V1x4S	4
#define V1x6S	6
#define ACCEL_SCALE 1
#define GYRO_SCALE 1146
#define GEOMAGNETIC_SCALE 1

template <typename TYPE>
class orientation_filter {
public:
	sensor_data<TYPE> m_accel;
	sensor_data<TYPE> m_gyro;
	sensor_data<TYPE> m_magnetic;
	vect<TYPE, MOVING_AVERAGE_WINDOW_LENGTH> m_var_gyr_x;
	vect<TYPE, MOVING_AVERAGE_WINDOW_LENGTH> m_var_gyr_y;
	vect<TYPE, MOVING_AVERAGE_WINDOW_LENGTH> m_var_gyr_z;
	vect<TYPE, MOVING_AVERAGE_WINDOW_LENGTH> m_var_roll;
	vect<TYPE, MOVING_AVERAGE_WINDOW_LENGTH> m_var_pitch;
	vect<TYPE, MOVING_AVERAGE_WINDOW_LENGTH> m_var_azimuth;
	matrix<TYPE, M6X6R, M6X6C> m_driv_cov;
	matrix<TYPE, M6X6R, M6X6C> m_aid_cov;
	matrix<TYPE, M6X6R, M6X6C> m_tran_mat;
	matrix<TYPE, M6X6R, M6X6C> m_measure_mat;
	matrix<TYPE, M6X6R, M6X6C> m_pred_cov;
	vect<TYPE, V1x6S> m_state_new;
	vect<TYPE, V1x6S> m_state_old;
	vect<TYPE, V1x6S> m_state_error;
	vect<TYPE, V1x3S> m_bias_correction;
	vect<TYPE, V1x3S> m_gyro_bias;
	quaternion<TYPE> m_quat_aid;
	quaternion<TYPE> m_quat_driv;
	rotation_matrix<TYPE> m_rot_matrix;
	euler_angles<TYPE> m_orientation;
	quaternion<TYPE> m_quat_9axis;
	quaternion<TYPE> m_quat_gaming_rv;
	quaternion<TYPE> m_quaternion;
	quaternion<TYPE>  m_quat_output;
	euler_angles<TYPE> m_euler_error;
	TYPE m_gyro_dt;

	orientation_filter(void);
	~orientation_filter(void);

	inline void initialize_sensor_data(const sensor_data<TYPE> *accel,
			const sensor_data<TYPE> *gyro, const sensor_data<TYPE> *magnetic);
	inline void orientation_triad_algorithm(void);
	inline void compute_accel_orientation(void);
	inline void compute_covariance(void);
	inline void time_update(void);
	inline void time_update_gaming_rv(void);
	inline void measurement_update(void);

	void get_device_orientation(const sensor_data<TYPE> *accel,
			const sensor_data<TYPE> *gyro, const sensor_data<TYPE> *magnetic);
};

#endif /* _ORIENTATION_FILTER_H_ */
