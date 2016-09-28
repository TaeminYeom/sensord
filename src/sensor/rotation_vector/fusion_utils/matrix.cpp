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

#ifdef _MATRIX_H_

TYPE_ROW_COL matrix<TYPE, ROW, COL>::matrix(void)
{
	for (int i = 0; i < ROW; i++)
		for (int j = 0; j < COL; j++)
			m_mat[i][j] = 0;
}

TYPE_ROW_COL matrix<TYPE, ROW, COL>::matrix(const matrix<TYPE, ROW, COL>& m)
{
	for (int p = 0; p < ROW; p++)
		for (int q = 0; q < COL; q++)
			m_mat[p][q] = m.m_mat[p][q];
}

TYPE_ROW_COL matrix<TYPE, ROW, COL>::matrix(TYPE mat_data[ROW][COL])
{
	for (int i = 0; i < ROW; i++)
		for (int j = 0; j < COL; j++)
			m_mat[i][j] = mat_data[i][j];
}

TYPE_ROW_COL matrix<TYPE, ROW, COL>::~matrix()
{
}

TYPE_ROW_COL matrix<TYPE, ROW, COL> matrix<TYPE, ROW, COL>::operator =(const matrix<TYPE, ROW, COL>& m)
{
	if (this == &m)
	{
		return *this;
	}

	for (int i = 0; i < ROW; i++)
		for (int j = 0; j < COL; j++)
			m_mat[i][j] = m.m_mat[i][j];

	return *this;
}

T_R_C ostream& operator <<(ostream& dout, matrix<T, R, C>& m)
{
	for (int i = 0; i < R; i++)
	{
		for (int j = 0; j < C; j++)
		{
			dout << m.m_mat[i][j] << "\t";
		}
		dout << endl;
	}
	return dout;
}

T_R_C matrix<T, R, C> operator +(const matrix<T, R, C> m1, const matrix<T, R, C> m2)
{
	matrix<T, R, C> m3;

	for (int i = 0; i < R; i++)
		for (int j = 0; j < C; j++)
			m3.m_mat[i][j] = m1.m_mat[i][j] + m2.m_mat[i][j];

	return m3;
}

T_R_C matrix<T, R, C> operator +(const matrix<T, R, C> m, const T val)
{
	matrix<T, R, C> m1;

	for (int i = 0; i < R; i++)
		for (int j = 0; j < C; j++)
			m1.m_mat[i][j] = m.m_mat[i][j] + val;

	return m1;
}

T_R_C matrix<T, R, C> operator -(const matrix<T, R, C> m1, const matrix<T, R, C> m2)
{
	matrix<T, R, C> m3;

	for (int i = 0; i < R; i++)
		for (int j = 0; j < C; j++)
			m3.m_mat[i][j] = m1.m_mat[i][j] - m2.m_mat[i][j];

	return m3;
}

T_R_C matrix<T, R, C> operator -(const matrix<T, R, C> m, const T val)
{
	matrix<T, R, C> m1;

	for (int i = 0; i < R; i++)
		for (int j = 0; j < C; j++)
			m1.m_mat[i][j] = m.m_mat[i][j] - val;

	return m1;
}

T_R_C_C2 matrix<T, R, C2> operator *(const matrix<T, R, C> m1, const matrix<T, C, C2> m2)
{
	matrix<T, R, C2> m3;

	for (int i = 0; i < R; i++)
	{
		for (int j = 0; j < C2; j++)
		{
			m3.m_mat[i][j] = 0;
			for (int k = 0; k < C; k++)
				m3.m_mat[i][j] += m1.m_mat[i][k] * m2.m_mat[k][j];
		}
	}

	return m3;
}

T_R_C matrix<T, R, C> operator *(const matrix<T, R, C> m, const T val)
{
	matrix<T, R, C> m1;

	for (int i = 0; i < R; i++)
		for (int j = 0; j < C; j++)
			m1.m_mat[i][j] = m.m_mat[i][j] * val;

	return m1;
}

T_R_C matrix<T, R, C> operator /(const matrix<T, R, C> m1, const T val)
{
	matrix<T, R, C> m3;

	for (int i = 0; i < R; i++)
		for (int j = 0; j < C; j++)
			m3.m_mat[i][j] = m1.m_mat[i][j] / val;

	return m3;
}

T_R1_C1_R2_C2 bool operator ==(const matrix<T, R1, C1> m1, const matrix<T, R2, C2> m2)
{
	if ((R1 == R2) && (C1 == C2))
	{
		for (int i = 0; i < R1; i++)
			for (int j = 0; j < C2; j++)
				if (m1.m_mat[i][j] != m2.m_mat[i][j])
					return false;
	}
	else
		return false;

	return true;
}

T_R1_C1_R2_C2 bool operator !=(const matrix<T, R1, C1> m1, const matrix<T, R2, C2> m2)
{
	return (!(m1 == m2));
}

T_R_C matrix<T, R, C> tran(const matrix<T, R, C> m)
{
	matrix<T, R, C> m1;

	for (int i = 0; i < R; i++)
		for (int j = 0; j < C; j++)
			m1.m_mat[j][i] = m.m_mat[i][j];

	return m1;
}

#endif //_MATRIX_H_
