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

#include "../../../euler_angles.h"

int main()
{
	float arr0[3] = {234.98, 345.24, -56.12};
	float arr1[3] = {56, -34, 76};
	float arr2[4] = {0.6, 0.6, -.18, -.44};
	float arr3[4] = {-0.5, -0.36, .43, .03};

	vect<float, 3> v0(arr0);
	vect<float, 3> v1(arr1);
	vect<float, 4> v2(arr2);
	vect<float, 4> v3(arr3);

	quaternion<float> q1(v2);
	quaternion<float> q2(v3);

	euler_angles<float> e0(v0);
	euler_angles<float> e1(v1);
	euler_angles<float> e2((float)234.98, (float)345.24, (float)-56.12);
	euler_angles<float> e3(e1);
	euler_angles<float> e4;

	cout << "Constructor tests\n";
	cout << "input\t" << v0 << "\n";
	cout << "output\t" << e0.m_ang << "\n\n";
	cout << "input\t" << v1 << "\n";
	cout << "output\t" << e1.m_ang << "\n\n";
	cout << "input\t" << v0 << "\n";
	cout << "output\t" << e2.m_ang << "\n\n";
	cout << "input\t" << v1 << "\n";
	cout << "output\t" << e3.m_ang << "\n\n";
	cout << "default constructor\n";
	cout << "output\t" << e4.m_ang << "\n\n";

	cout << "Quaternion to Euler\n";
	euler_angles<float> e5 = quat2euler(q1);
	cout << "input\t" << q1.m_quat << "\n";
	cout << "output\t" << e5.m_ang << "\n\n";
	euler_angles<float> e8 = quat2euler(q2);
	cout << "input\t" << q2.m_quat << "\n";
	cout << "output\t" << e8.m_ang << "\n\n";

	cout << "Euler to Quaternion\n";
	quaternion<float> q3 = euler2quat(e8);
	cout << "input\t" << e8.m_ang << "\n";
	cout << "output\t" << q3.m_quat << "\n\n";

	cout << "Radians to Degrees\n";
	euler_angles<float> e6 = deg2rad(e0);
	cout << "input\t" << e0.m_ang << "\n";
	cout << "output\t" << e6.m_ang << "\n\n";

	cout << "Degrees to Radians\n";
	euler_angles<float> e7 = rad2deg(e6);
	cout << "input\t" << e6.m_ang << "\n";
	cout << "output\t" << e7.m_ang << "\n\n";
}

