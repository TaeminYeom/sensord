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

#if defined (_SENSOR_DATA_H_) && defined (_VECTOR_H_)

#include "math.h"

template <typename TYPE>
sensor_data<TYPE>::sensor_data() : m_data(), m_time_stamp(0)
{
}

template <typename TYPE>
sensor_data<TYPE>::sensor_data(const TYPE x, const TYPE y,
		const TYPE z, const unsigned long long time_stamp)
{
	TYPE vec_data[SENSOR_DATA_SIZE] = {x, y, z};

	vect<TYPE, SENSOR_DATA_SIZE> v(vec_data);
	m_data = v;
	m_time_stamp = time_stamp;
}

template <typename TYPE>
sensor_data<TYPE>::sensor_data(const vect<TYPE, SENSOR_DATA_SIZE> v,
		const unsigned long long time_stamp)
{
	m_data = v;
	m_time_stamp = time_stamp;
}

template <typename TYPE>
sensor_data<TYPE>::sensor_data(const sensor_data<TYPE>& s)
{
	m_data = s.m_data;
	m_time_stamp = s.m_time_stamp;
}

template <typename TYPE>
sensor_data<TYPE>::~sensor_data()
{
}

template <typename TYPE>
sensor_data<TYPE> sensor_data<TYPE>::operator =(const sensor_data<TYPE>& s)
{
	m_data = s.m_data;
	m_time_stamp = s.m_time_stamp;

	return *this;
}

template <typename T>
sensor_data<T> operator +(sensor_data<T> data1, sensor_data<T> data2)
{
	sensor_data<T> result(data1.m_data + data2.m_data, 0);
	return result;
}

template <typename T>
void normalize(sensor_data<T>& data)
{
	T x, y, z;

	x = data.m_data.m_vec[0];
	y = data.m_data.m_vec[1];
	z = data.m_data.m_vec[2];

	T val = sqrt(x*x + y*y + z*z);

	data.m_data.m_vec[0] = x / val;
	data.m_data.m_vec[1] = y / val;
	data.m_data.m_vec[2] = z / val;
}

template <typename T>
sensor_data<T> scale_data(sensor_data<T> data, T scaling_factor)
{
	T x, y, z;

	x = data.m_data.m_vec[0] / scaling_factor;
	y = data.m_data.m_vec[1] / scaling_factor;
	z = data.m_data.m_vec[2] / scaling_factor;

	sensor_data<T> s(x, y, z, data.m_time_stamp);

	return s;
}


template<typename T>
quaternion<T> sensor_data2quat(const sensor_data<T> data, const vect<T, REF_VEC_SIZE> ref_vec)
{
	vect<T, REF_VEC_SIZE> axis;
	T angle;

	axis = cross(data.m_data, ref_vec);
	angle = acos(dot(data.m_data, ref_vec));

	return axis2quat(axis, angle);
}

template<typename T>
void pre_process_data(sensor_data<T> &data_out, const T *data_in, int sign, int scale)
{
	data_out.m_data.m_vec[0] = sign * data_in[0] / scale;
	data_out.m_data.m_vec[1] = sign * data_in[1] / scale;
	data_out.m_data.m_vec[2] = sign * data_in[2] / scale;
}

#endif /* _SENSOR_DATA_H_ */
