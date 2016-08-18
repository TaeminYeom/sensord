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

#include "../../../vector.h"

int main()
{
	float arr2[3] = {0.056, 2.34, -65.76};
	float arr15[3] = {1.04, -4.678, -2.34};
	float arr3[4] = {1.03, 2.345, 6.78, 5.55};
	float arr4[4] = {-6.78, -45.213, -7.89, -3.456};
	float arr8[5] = {0.0123, 5.678, 2.345, 0.345, 8.956};
	float arr0[5] = {2344.98, 345.24, 456.12, 98.33, 144.67};
	float arr1[6] = {1.234, 4.5, 6.8987, 3.33, 5.66, 77.695};
	float arr43[6] = {2.3454, -0.0384, -8.90, 3.455, 6.785, 21.345};
	float arr5[5] = {0.2, -0.4, 0.6, -0.8, 1.0};

	vect<float, 5> v1(arr0);
	vect<float, 5> v2(arr8);
	vect<float, 4> v10(arr3);
	vect<float, 4> v12(arr4);
	vect<float, 6> v15(arr1);
	vect<float, 6> v20(arr43);
	vect<float, 3> v21(arr2);
	vect<float, 3> v22(arr15);
	vect<float, 5> v31(arr0);
	vect<float, 3> v3;
	vect<float, 2> vn;
	vect<float, 4> v61(arr4);
	vect<float, 3> v6;
	vect<float, 4> vm;
	vect<float, 5> v13;
	vect<float, 6> v95;
	vect<float, 5> v35(arr5);
	vect<float, 5> vl;
	vect<float, 4> vp;
	vect<float, 4> vr(arr4);
	vect<float, 3> vf;

	float arr57[3][3] = {{2.24, 0.5, 0.023}, {3.675, 5.32, 0.556}, {1.023, 45.75, 621.6}};
	matrix<float, 3, 3> m12(arr57);
	float arr67[3][1] = {{2.0}, {3.0}, {4.0}};
	matrix<float, 3, 1> m32(arr67);

	cout << "Constructor Test\n";
	cout << "\n" << v3;

	cout << "\n\nAddition\n";
	v3 = v21 + v22;
	v95 = v15 + v20;
	cout << "\n\nNumbers added\n";
	cout << "\n" << v21 << "\n" << v22;
	cout << "\nSum:\n" << v3 << endl;
	cout << "\n\nNumbers added\n";
	cout << "\n" << v15 << "\n" << v20;
	cout << "\nSum:\n" << v95 << endl;

	float num = 5.5650;
	float num1 = -2.32;
	cout << "\n\n\nScalar addition\n";
	vl = v2 + num;
	vm = v10 + num1;
	cout << "\nNumber added:" << num;
	cout << "\n\n" << v2;
	cout << "\nResult:\n\n" << vl;
	cout << "\nNumber added:" << num1;
	cout << "\n\n" << v10;
	cout << "\nResult:\n\n" << vm;

	cout << "\n\n\nSubtraction\n";
	vp = v10 - v12;
	cout << "\n" << v10 << "\n" << v12;
	cout << "\nDifference:\n" << vp << endl;

	float x = 4.0;
	float x1 = -2.5;
	cout << "\n\n\nScalar subtraction\n";
	v13 = v1 - x;
	vp = v10 - x1;
	cout << "\nNumber Subtracted:" << x;
	cout << "\n\n" << v1;
	cout << "\nResult:\n\n" << v13;
	cout << "\nNumber Subtracted:" << x1;
	cout << "\n\n" << v10;
	cout << "\nResult:\n\n" << vp;

	float xx = 7.2;
	cout << "\n\n\nMultiplication\n";
	v13 = v2 * xx;
	cout << "\n" << v2 << "\n" << xx;
	cout << "\nProduct:\n" << v13 << endl;

	cout << "\n\n\nMultiplication matrix x vector:\n";
	matrix<float, 3, 3> m102 = m32 * v22;
	cout << "\n" << m32 << "\n" << v22;
	cout << "\nProduct:\n" << m102 << endl;

	cout << "\n\n\nVector x Multiplication matrix:\n";
	vect<float, 3> v102 = (v22 * m12);
	cout << "\n" << v22 << "\n" << m12;
	cout << "\nProduct:\n" << v102 << endl;

	float z = 3.50;
	float z1 = -5.567;
	cout << "\n\n\nScalar multiplication\n";
	v13 = v1 * z;
	v61 = v12 * z1;
	cout << "\nNumber Multiplied:" << z;
	cout << "\n\n" << v1;
	cout << "\nResult:\n\n" << v13;
	cout << "\nNumber Multiplied:" << z1;
	cout << "\n\n" << v12;
	cout << "\nResult:\n\n" << v6;

	float num2 = 5.5;
	cout << "\n\n\nDivision\n";
	v31 = v1 / num2;
	cout << "\n" << v1 << "\n" << num2;
	cout << "\nResult:\n" << v3 << endl;

	cout << "\n\n\nTranspose:";
	cout << "\n\n" << v20;
	cout << "\nResult:\n\n";
	matrix<float, 6, 1> m101 = transpose(v20);
	cout << m101;
	cout << "\n\n" << m101;
	cout << "\nResult:\n\n";
	v20 = transpose(m101);
	cout << v20;

	cout << "\n\nv1:\n\n" << v1;
	cout << "\n\nv2:\n\n" << v2;
	cout << "\n\n\nv1 == v2 :";
	cout << (v1 == v2);

	cout << "\n\nv10:\n\n" << v10;
	cout << "\n\nv10:\n\n" << v10;
	cout << "\n\n\nv10 == v10 :";
	cout << (v10 == v10);

	cout << "\n\nv12:\n\n" << v12;
	cout << "\n\nv15:\n\n" << vr;
	cout << "\n\n\nv12 != v15 :";
	cout << (v12 != vr);

	cout << "\n\nv15:\n\n" << v15;
	cout << "\n\nv15:\n\n" << v15;
	cout << "\n\n\nv15 != v15 :";
	cout << (v15 != v15);

	cout << "\n\nAssignment\n";
	v3 = vf;
	cout << "Input \n" << v1;
	cout << "\nOutput:\n" << v3 << endl;

	vect<float, 3> v111 = cross(v21, v22);
	cout << "\n\n\nCross Product:";
	cout << "\n\n" << v21 << "\n\n" << v22;
	cout << "\nResult:\n\n" << v111;

	float val = dot(v21, v22);
	cout << "\n\n\nDot Product:";
	cout << "\n\n" << v21 << "\n\n" << v22;
	cout << "\nResult:\n\n" << val;

	cout <<  "\n\n\nQueue insert function:";
	cout << "\nInput:\n\n" << v111;
	insert_end(v111, (float) 0.9191919);
	cout << "\nResult:\n\n" << v111;

	cout <<  "\n\n\nVariance:";
	cout << "\nInput:\n\n" << v35;
	val = var(v35);
	cout << "\nResult:\n\n" << val;

	cout <<  "\n\n\nIs Initialized:";
	cout << "\nInput:\n\n" << v35;
	cout << "\nResult:\n\n" << is_initialized(v35) << endl;
}

