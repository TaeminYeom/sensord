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

#ifndef _VECTOR_H_
#define _VECTOR_H_

#include "matrix.h"

#define TYPE_SIZE template<typename TYPE, int SIZE>
#define T_S template<typename T, int S>
#define T_S_R_C template<typename T, int S, int R, int C>
#define T_S1_S2 template<typename T, int S1, int S2>

TYPE_SIZE class vect {
public:
	TYPE m_vec[SIZE];
	vect(void);
	vect(TYPE vec_data[SIZE]);
	vect(const vect<TYPE, SIZE>& v);
	~vect();

	vect<TYPE, SIZE> operator =(const vect<TYPE, SIZE>& v);

	T_S friend ostream& operator << (ostream& dout, vect<T, S>& v);
	T_S friend vect<T, S> operator +(const vect<T, S> v1, const vect<T, S> v2);
	T_S friend vect<T, S> operator +(const vect<T, S> v, const T val);
	T_S friend vect<T, S> operator -(const vect<T, S> v1, const vect<T, S> v2);
	T_S friend vect<T, S> operator -(const vect<T, S> v, const T val);
	T_S_R_C friend matrix<T, R, S> operator *(const matrix<T, R, C> v1, const vect<T, S> v2);
	T_S_R_C friend vect<T, S> operator *(const vect<T, S> v, const matrix<T, R, C> m);
	T_S friend vect<T, S> operator *(const vect<T, S> v, const T val);
	T_S friend vect<T, S> operator /(const vect<T, S> v1, const T val);
	T_S1_S2 friend bool operator ==(const vect<T, S1> v1, const vect<T, S2> v2);
	T_S1_S2 friend bool operator !=(const vect<T, S1> v1, const vect<T, S2> v2);

	T_S friend void insert_end(vect<T, S>& v, T val);
	T_S friend matrix<T, S, 1> transpose(const vect<T, S> v);
	T_S friend vect<T, S> transpose(const matrix<T, S, 1> m);
	T_S friend vect<T, S> cross(const vect<T, S> v1, const vect<T, S> v2);
	T_S friend T dot(const vect<T, S> v1, const vect<T, S> v2);
	T_S friend T var(const vect<T, S> v);
	T_S friend bool is_initialized(const vect<T, S> v);
};

#include "vector.cpp"

#endif /* _VECTOR_H_ */

