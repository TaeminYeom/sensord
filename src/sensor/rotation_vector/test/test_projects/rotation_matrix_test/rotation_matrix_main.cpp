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

#include "../../../rotation_matrix.h"

int main()
{
	float arr1[3][3] = {{20.2, 40.5, 10.0}, {3.6, 52.0, 5.5}, {1.0, 45.5, 66.6}};
	float arr2[3][3] = {{2.24, 0.5, 0.023}, {3.675, 5.32, 0.556}, {1.023, 45.75, 621.6}};
	float arr3[3][3] = {{4.75, 0.65, 0.123}, {0.075, 5.302, 0.56}, {1.113, 0.475, 2.362}};

	matrix<float, 3, 3> m1(arr1);
	matrix<float, 3, 3> m2(arr2);
	matrix<float, 3, 3> m3(arr3);

	rotation_matrix<float> rm0, rm5;
	rotation_matrix<float> rm1(m1);
	rotation_matrix<float> rm2(m2);
	rotation_matrix<float> rm3(m3);
	rotation_matrix<float> rm4(arr1);

	quaternion<float> q0(-0.612372, 0.456436, 0.456436, 0.456436);
	quaternion<float> q1;

	cout << "Constructor tests\n";
	cout << "input\n" << m1 << "\n";
	cout << "output\n" << rm1.m_rot_mat << "\n\n";
	cout << "input\n" << m2 << "\n";
	cout << "output\n" << rm2.m_rot_mat << "\n\n";
	cout << "input\n" << m3 << "\n";
	cout << "output\n" << rm3.m_rot_mat << "\n\n";
	cout << "input\n" << m1 << "\n";
	cout << "output\n" << rm4.m_rot_mat << "\n\n";
	cout << "default constructor\n";
	cout << "output\n" << rm0.m_rot_mat << "\n\n";

	cout << "Quaternion to Rotation Matrix\n";
	cout << "input\n" << q0.m_quat << "\n";
	rm0 = quat2rot_mat(q0);
	cout << "output\n" << rm0.m_rot_mat << "\n\n";

	cout << "Rotation Matrix to Quaternion\n";
	cout << "input\n" << rm0.m_rot_mat << "\n";
	q1 = rot_mat2quat(rm0);
	cout << "output\n" << q1.m_quat << "\n\n";
}
