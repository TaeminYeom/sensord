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

#ifndef _QUATERNION_H_
#define _QUATERNION_H_

#include "vector.h"

#define QUAT_SIZE 4
#define REF_VEC_SIZE 3

template <typename TYPE>
class quaternion {
public:
	vect<TYPE, QUAT_SIZE> m_quat;

	quaternion();
	quaternion(const TYPE w, const TYPE x, const TYPE y, const TYPE z);
	quaternion(const vect<TYPE, QUAT_SIZE> v);
	quaternion(const quaternion<TYPE>& q);
	~quaternion();

	quaternion<TYPE> operator =(const quaternion<TYPE>& q);
	void quat_normalize(void);

	template<typename T> friend quaternion<T> operator *(const quaternion<T> q,
			const T val);
	template<typename T> friend quaternion<T> operator *(const quaternion<T> q1,
			const quaternion<T> q2);
	template<typename T> friend quaternion<T> operator +(const quaternion<T> q1,
			const quaternion<T> q2);
	template<typename T> friend quaternion<T> phase_correction(const quaternion<T> q1,
			const quaternion<T> q2);
	template<typename T> friend quaternion<T> axis2quat(const vect<T, REF_VEC_SIZE> axis,
			const T angle);
};

#include "quaternion.cpp"

#endif  //_QUATERNION_H_
