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

#ifndef _MATRIX_H_
#define _MATRIX_H_

#include <assert.h>
#include <iostream>
using namespace std;

#define TYPE_ROW_COL template<typename TYPE, int ROW, int COL>
#define T_R_C template<typename T, int R, int C>
#define T_R_C_C2 template<typename T, int R, int C, int C2>
#define T_R1_C1_R2_C2 template<typename T, int R1, int C1, int R2, int C2>

TYPE_ROW_COL class matrix {
public:
	TYPE m_mat[ROW][COL];

	matrix(void);
	matrix(TYPE mat_data[ROW][COL]);
	matrix(const matrix<TYPE, ROW, COL>& m);
	~matrix();

	matrix<TYPE, ROW, COL> operator =(const matrix<TYPE, ROW, COL>& m);

	T_R_C friend ostream& operator << (ostream& dout, matrix<T, R, C>& m);
	T_R_C friend matrix<T, R, C> operator +(const matrix<T, R, C> m1, const matrix<T, R, C> m2);
	T_R_C friend matrix<T, R, C> operator +(const matrix<T, R, C> m, const T val);
	T_R_C friend matrix<T, R, C> operator -(const matrix<T, R, C> m1, const matrix<T, R, C> m2);
	T_R_C friend matrix<T, R, C> operator -(const matrix<T, R, C> m, const T val);
	T_R_C_C2 friend matrix<T, R, C2> operator *(const matrix<T, R, C> m1, const matrix<T, C, C2> m2);
	T_R_C friend matrix<T, R, C> operator *(const matrix<T, R, C> m, const T val);
	T_R_C friend matrix<T, R, C> operator /(const matrix<T, R, C> m1, const T val);
	T_R1_C1_R2_C2 friend bool operator ==(const matrix<T, R1, C1> m1, const matrix<T, R2, C2> m2);
	T_R1_C1_R2_C2 friend bool operator !=(const matrix<T, R1, C1> m1, const matrix<T, R2, C2> m2);
	T_R_C friend matrix<T, R, C> tran(const matrix<T, R, C> m);
};

#include "matrix.cpp"

#endif  //_MATRIX_H_
