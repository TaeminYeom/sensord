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

#include "../../../matrix.h"

int main()
{
	float arr0[2][2] = {{-2.243, 2.57}, {3.56, -3.02}};
	float arr1[2][2] = {{2.2, 2.5}, {3.5, 3.0}};
	float arr5[3][2] = {{0.22, 4.56}, {3.652, 5.86}, {1.430, 0.45}};
	float arr11[1][3] = {{2.0, 3.0, 4.0}};
	float arr12[3][1] = {{2.0}, {1.0}, {3.0}};
	float arr15[2][3] = {{20.0, -40.0, 10.0}, {36.0, 52.0, -55.0}};
	float arr3[3][3] = {{20.2, 40.5, 10.0}, {3.6, 52.0, 5.5}, {1.0, 45.5, 66.6}};
	float arr4[3][3] = {{2.24, 0.5, 0.023}, {3.675, 5.32, 0.556}, {1.023, 45.75, 621.6}};
	float arr8[3][3] = {{4.75, 0.65, 0.123}, {0.075, 5.302, 0.56}, {1.113, 0.475, 2.362}};
	float arr9[3][3] = {{1, 2, 3}, {1, 2, 3}, {1, 2, 3}};

	matrix<float, 2, 2> m1(arr0);
	matrix<float, 2, 2> m2(arr1);
	matrix<float, 2, 2> m3;
	matrix<float, 3, 3> m10(arr3);
	matrix<float, 3, 2> m11(arr5);
	matrix<float, 3, 3> m6;
	matrix<float, 3, 2> m13;
	matrix<float, 3, 3> m12(arr4);
	matrix<float, 3, 3> m15(arr8);
	matrix<float, 1, 3> m20(arr11);
	matrix<float, 3, 1> m21(arr12);
	matrix<float, 2, 3> m22(arr15);
	matrix<float, 3, 3> m9(arr9);

	cout << "Constructor Test\n";
	cout << "\n" << m6;

	cout << "\n\nAddition\n";
	m6 = m10 + m15;
	m13 = m11 + m11;
	cout << "\n" << m10 << "\n" << m15;
	cout << "\nSum:\n" << m6 << endl;
	cout << "\n" << m11 << "\n" << m11;
	cout << "\nSum:\n" << m13 << endl;

	cout << "\n\n\nSubtraction\n";
	m6 = m10 - m12;
	cout << "\n" << m10 << "\n" << m12;
	cout << "\nDifference:\n" << m6 << endl;

	cout << "\n\n\nMultiplication\n";
	m6 = m10 * m12;
	m3 = m1 * m2;
	matrix<float, 1, 1> m7;
	m7 = m20 * m21;
	cout << "\n" << m10 << "\n" << m12;
	cout << "\nProduct:\n" << m6 << endl;
	cout << "\n" << m1 << "\n" << m2;
	cout << "\nProduct:\n" << m3 << endl;
	cout << "\n" << m20 << "\n" << m21;
	cout << "\nProduct:\n" << m7 << endl;
	cout << "\n" << m9 << "\n" << m21;
	m21 = m9 * m21;
	cout << "\nProduct:\n" << m21 << endl;

	cout << "\n\n\nDivision\n";
	m3 = m1 / (float)2.5;
	cout << "\n" << m1 << "\n" << "2.5";
	cout << "\nResult:\n" << m3 << endl;
	m6 = m12 / (float)0.125;
	cout << "\n" << m12 << "\n" << ".125";
	cout << "\nResult:\n" << m6 << endl;

	float num = 5.5650;
	float num1 = -2.32;
	cout << "\n\n\nScalar addition\n";
	m3 = m2 + num;
	m6 = m10 + num1;
	cout << "\nNumber added:" << num;
	cout << "\n\n" << m2;
	cout << "\nResult:\n\n" << m3;
	cout << "\nNumber added:" << num1;
	cout << "\n\n" << m10;
	cout << "\nResult:\n\n" << m6;

	float x = 4.0;
	float x1 = -2.5;
	cout << "\n\n\nScalar subtraction\n";
	m13 = m11 - x;
	m6 = m10 - x1;
	cout << "\nNumber Subtracted:" << x;
	cout << "\n\n" << m11;
	cout << "\nResult:\n\n" << m13;
	cout << "\nNumber Subtracted:" << x1;
	cout << "\n\n" << m10;
	cout << "\nResult:\n\n" << m6;

	float z = 3.50;
	float z1 = -5.567;
	cout << "\n\n\nScalar multiplication\n";
	m3 = m1 * z;
	m6 = m12 * z1;
	cout << "\nNumber Multiplied:" << z;
	cout << "\n\n" << m1;
	cout << "\nResult:\n\n" << m3;
	cout << "\nNumber Multiplied:" << z1;
	cout << "\n\n" << m12;
	cout << "\nResult:\n\n" << m6;

	m6 = tran(m15);
	cout << "\n\n\nTranspose:";
	cout << "\n\n" << m15;
	cout << "\nResult:\n\n" << m6;

	cout << "\n\nm1:\n\n" << m1;
	cout << "\n\nm2:\n\n" << m2;
	cout << "\n\n\nm1 == m2 :";
	cout << (m1 == m2);

	cout << "\n\nm2:\n\n" << m2;
	cout << "\n\nm2:\n\n" << m2;
	cout << "\n\n\nm2 == m2 :";
	cout << (m2 == m2);

	cout << "\n\nm6:\n\n" << m6;
	cout << "\n\nm6:\n\n" << m6;
	cout << "\n\n\nm6 != m6 :";
	cout << (m6 != m6);

	cout << "\n\nm6:\n\n" << m6;
	cout << "\n\nm1:\n\n" << m1;
	cout << "\n\n\nm6 != m1 :";
	cout << (m6 != m1);


	cout << "\n\nAssignment\n";
	matrix<float, 3, 3> m30 = m12;
	cout << "Input \n" << m12;
	cout << "\nOutput:\n" << m30 << endl;
}
