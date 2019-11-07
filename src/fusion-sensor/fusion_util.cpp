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
#include <errno.h>
#include <math.h>
#include <stdlib.h>

#include "fusion_util.h"

#define RAD2DEGREE (180/M_PI)
#define QUAT (M_PI/4)
#define HALF (M_PI/2)
#define ARCTAN(x, y) ((x) == 0 ? 0 : (y) != 0 ? atan2((x), (y)) : (x) > 0 ? M_PI/2.0 : -M_PI/2.0)

static float clamp(float v)
{
	return (v < 0) ? 0.0 : v;
}

int quat_to_matrix(const float *quat, float *R)
{
	if (quat == NULL || R == NULL)
		return -EINVAL;

	float q0 = quat[3];
	float q1 = quat[0];
	float q2 = quat[1];
	float q3 = quat[2];

	float sq_q1 = 2 * q1 * q1;
	float sq_q2 = 2 * q2 * q2;
	float sq_q3 = 2 * q3 * q3;
	float q1_q2 = 2 * q1 * q2;
	float q3_q0 = 2 * q3 * q0;
	float q1_q3 = 2 * q1 * q3;
	float q2_q0 = 2 * q2 * q0;
	float q2_q3 = 2 * q2 * q3;
	float q1_q0 = 2 * q1 * q0;

	R[0] = 1 - sq_q2 - sq_q3;
	R[1] = q1_q2 - q3_q0;
	R[2] = q1_q3 + q2_q0;
	R[3] = q1_q2 + q3_q0;
	R[4] = 1 - sq_q1 - sq_q3;
	R[5] = q2_q3 - q1_q0;
	R[6] = q1_q3 - q2_q0;
	R[7] = q2_q3 + q1_q0;
	R[8] = 1 - sq_q1 - sq_q2;

	return 0;
}

int matrix_to_quat(const float *R, float *quat)
{
	if (R == NULL || quat == NULL)
		return -EINVAL;

	const float Hx = R[0];
	const float My = R[4];
	const float Az = R[8];
	quat[0] = sqrtf(clamp(Hx - My - Az + 1) * 0.25f);
	quat[1] = sqrtf(clamp(-Hx + My - Az + 1) * 0.25f);
	quat[2] = sqrtf(clamp(-Hx - My + Az + 1) * 0.25f);
	quat[3] = sqrtf(clamp(Hx + My + Az + 1) * 0.25f);
	quat[0] = copysignf(quat[0], R[7] - R[5]);
	quat[1] = copysignf(quat[1], R[2] - R[6]);
	quat[2] = copysignf(quat[2], R[3] - R[1]);

	return 0;
}

int calculate_rotation_matrix(float *accel, float *geo, float *R, float *I)
{
	if (accel == NULL || geo == NULL || R == NULL || I == NULL)
		return -EINVAL;

	float Ax = accel[0];
	float Ay = accel[1];
	float Az = accel[2];
	float Ex = geo[0];
	float Ey = geo[1];
	float Ez = geo[2];
	float Hx = Ey*Az - Ez*Ay;
	float Hy = Ez*Ax - Ex*Az;
	float Hz = Ex*Ay - Ey*Ax;
	float normH =  (float)sqrt(Hx*Hx + Hy*Hy + Hz*Hz);
	if (normH < 0.1f)
		return -EINVAL;

	float invH = 1.0f / normH;
	Hx *= invH;
	Hy *= invH;
	Hz *= invH;
	float invA = 1.0f / (float)sqrt(Ax*Ax + Ay*Ay + Az*Az);
	Ax *= invA;
	Ay *= invA;
	Az *= invA;
	float Mx = Ay*Hz - Az*Hy;
	float My = Az*Hx - Ax*Hz;
	float Mz = Ax*Hy - Ay*Hx;

	R[0] = Hx;  R[1] = Hy;  R[2] = Hz;
	R[3] = Mx;  R[4] = My;  R[5] = Mz;
	R[6] = Ax;  R[7] = Ay;	R[8] = Az;

	float invE = 1.0 / (float)sqrt(Ex*Ex + Ey*Ey + Ez*Ez);
	float c = (Ex*Mx + Ey*My + Ez*Mz) * invE;
	float s = (Ex*Ax + Ey*Ay + Ez*Az) * invE;

	I[0] = 1;     I[1] = 0;     I[2] = 0;
	I[3] = 0;     I[4] = c;     I[5] = s;
	I[6] = 0;     I[7] = -s;    I[8] = c;

	return 0;
}

int quat_to_orientation(const float *quat, float &azimuth, float &pitch, float &roll)
{
	int error;
	float g[3];
	float R[9];

	error = quat_to_matrix(quat, R);

	if (error < 0)
		return error;

	float xyz_z = ARCTAN(R[3], R[0]);
	float yxz_x = asinf(R[7]);
	float yxz_y = ARCTAN(-R[6], R[8]);
	float yxz_z = ARCTAN(-R[1], R[4]);

	float a = fabs(yxz_x / HALF);
	a = a * a;

	float p = (fabs(yxz_y) / HALF - 1.0);

	if (p < 0)
		p = 0;

	float v = 1 + (1 - a) / a * p;

	if (v > 20)
		v = 20;

	if (yxz_x * yxz_y > 0) {
		if (yxz_z > 0 && xyz_z < 0)
			xyz_z += M_PI * 2;
	} else {
		if (yxz_z < 0 && xyz_z > 0)
			xyz_z -= M_PI * 2;
	}

	g[0] = (1 - a * v) * yxz_z + (a * v) * xyz_z;
	g[0] *= -1;

	float tmp = R[7];

	if (tmp > 1.0f)
		tmp = 1.0f;
	else if (tmp < -1.0f)
		tmp = -1.0f;

	g[1] = -asinf(tmp);
	if (R[8] < 0)
		g[1] = M_PI - g[1];

	if (g[1] > M_PI)
		g[1] -= M_PI * 2;

	if ((fabs(R[7]) > QUAT))
		g[2] = (float) atan2f(R[6], R[7]);
	else
		g[2] = (float) atan2f(R[6], R[8]);

	if (g[2] > HALF)
		g[2] = M_PI - g[2];
	else if (g[2] < -HALF)
		g[2] = -M_PI - g[2];

	g[0] *= RAD2DEGREE;
	g[1] *= RAD2DEGREE;
	g[2] *= RAD2DEGREE;

	if (g[0] < 0)
		g[0] += 360;

	azimuth = g[0];
	pitch = g[1];
	roll = g[2];

	return 0;
}

