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

#if defined (_VECTOR_H_) && defined (_MATRIX_H_)

TYPE_SIZE vect<TYPE, SIZE>::vect(void)
{
	for (int i = 0; i < SIZE; i++)
		m_vec[i] = 0;
}

TYPE_SIZE vect<TYPE, SIZE>::vect(TYPE vec_data[SIZE])
{
	for (int j = 0; j < SIZE; j++)
		m_vec[j] = vec_data[j];
}

TYPE_SIZE vect<TYPE, SIZE>::vect(const vect<TYPE, SIZE>& v)
{
	for (int q = 0; q < SIZE; q++)
		m_vec[q] = v.m_vec[q];
}


TYPE_SIZE vect<TYPE, SIZE>::~vect()
{
}

TYPE_SIZE vect<TYPE, SIZE> vect<TYPE, SIZE>::operator =(const vect<TYPE, SIZE>& v)
{
	if (this == &v)
		return *this;

	for (int q = 0; q < SIZE; q++)
		m_vec[q] = v.m_vec[q];

	return *this;
}

T_S ostream& operator <<(ostream& dout, vect<T, S>& v)
{
	for (int j = 0; j < S; j++)
		dout << v.m_vec[j] << "\t";

	dout << endl;

	return dout;
}

T_S vect<T, S> operator +(const vect<T, S> v1, const vect<T, S> v2)
{
	vect<T, S> v3;

	for (int j = 0; j < S; j++)
		v3.m_vec[j] = v1.m_vec[j] + v2.m_vec[j];

	return v3;
}

T_S vect<T, S> operator +(const vect<T, S> v, const T val)
{
	vect<T, S> v1;

	for (int j = 0; j < S; j++)
		v1.m_vec[j] = v.m_vec[j] + val;

	return v1;
}

T_S vect<T, S> operator -(const vect<T, S> v1, const vect<T, S> v2)
{
	vect<T, S> v3;

	for (int j = 0; j < S; j++)
		v3.m_vec[j] = v1.m_vec[j] - v2.m_vec[j];

	return v3;
}

T_S vect<T, S> operator -(const vect<T, S> v, const T val)
{
	vect<T, S> v1;

	for (int j = 0; j < S; j++)
		v1.m_vec[j] = v.m_vec[j] - val;

	return v1;
}

T_S_R_C matrix<T, R, S> operator *(const matrix<T, R, C> m, const vect<T, S> v)
{
	assert(R == S);
	assert(C == 1);

	matrix<T, R, S> m1;

	for (int i = 0; i < R; i++)
		for (int j = 0; j < S; j++)
			m1.m_mat[i][j] = m.m_mat[i][0] * v.m_vec[j];

	return m1;
}

T_S_R_C vect<T, S> operator *(const vect<T, S> v, const matrix<T, R, C> m)
{
	assert(R == S);
	assert(C != 1);
	vect<T, C> v1;

	for (int j = 0; j < C; j++)
	{
		v1.m_vec[j] = 0;
		for (int k = 0; k < R; k++)
			v1.m_vec[j] += v.m_vec[k] * m.m_mat[k][j];
	}

	return v1;
}

T_S vect<T, S> operator *(const vect<T, S> v, const T val)
{
	vect<T, S> v1;

	for (int j = 0; j < S; j++)
		v1.m_vec[j] = v.m_vec[j] * val;

	return v1;
}

T_S vect<T, S> operator /(const vect<T, S> v, const T val)
{
	vect<T, S> v1;

	for (int j = 0; j < S; j++)
		v1.m_vec[j] = v.m_vec[j] / val;

	return v1;
}

T_S1_S2 bool operator ==(const vect<T, S1> v1, const vect<T, S2> v2)
{
	if (S1 == S2)
	{
		for (int i = 0; i < S1; i++)
			if (v1.m_vec[i] != v2.m_vec[i])
				return false;
	}
	else
		return false;

	return true;
}

T_S1_S2 bool operator !=(const vect<T, S1> v1, const vect<T, S2> v2)
{
	return (!(v1 == v2));
}

T_S matrix<T, S, 1> transpose(const vect<T, S> v)
{
	matrix<T, S, 1> m;

	for (int i = 0; i < S; i++)
		m.m_mat[i][0] = v.m_vec[i];

	return m;
}

T_S vect<T, S> transpose(const matrix<T, S, 1> m)
{
	vect<T, S> v;

	for (int i = 0; i < S; i++)
		v.m_vec[i] = m.m_mat[i][0];

	return v;
}

T_S void insert_end(vect<T, S>& v, T val)
{
	for (int i = 0; i < (S - 1); i++)
		v.m_vec[i] = v.m_vec[i+1];

	v.m_vec[S-1] = val;
}

T_S vect<T, S> cross(const vect<T, S> v1, const vect<T, S> v2)
{
	vect<T, S> v3;

	v3.m_vec[0] = ((v1.m_vec[1] * v2.m_vec[2]) - (v1.m_vec[2] * v2.m_vec[1]));
	v3.m_vec[1] = ((v1.m_vec[2] * v2.m_vec[0]) - (v1.m_vec[0] * v2.m_vec[2]));
	v3.m_vec[2] = ((v1.m_vec[0] * v2.m_vec[1]) - (v1.m_vec[1] * v2.m_vec[0]));

	return v3;
}

T_S T dot(const vect<T, S> v1, const vect<T, S> v2)
{
	return (v1.m_vec[0] * v2.m_vec[0] + v1.m_vec[1] * v2.m_vec[1] + v1.m_vec[2] * v2.m_vec[2]);
}

T_S bool is_initialized(const vect<T, S> v)
{
	vect<T, S> v1;
	bool retval;

	retval = (v == v1) ? false : true;

	return retval;
}

T_S T var(const vect<T, S> v)
{
	T val = 0;
	T mean, var, diff;

	for (int i = 0; i < S; i++)
		val += v.m_vec[i];

	mean = val / S;

	val = 0;
	for (int i = 0; i < S; i++)
	{
		diff = (v.m_vec[i] - mean);
		val += diff * diff;
	}

	var = val / (S - 1);

	return var;
}
#endif

