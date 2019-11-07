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

#ifndef _FUSION_UTIL_H_
#define _FUSION_UTIL_H_

#ifdef __cplusplus
extern "C"
{
#endif

int quat_to_matrix(const float *quat, float *R);
int matrix_to_quat(const float *R, float *quat);
int calculate_rotation_matrix(float *accel, float *geo, float *R, float *I);
int quat_to_orientation(const float *rotation, float &azimuth, float &pitch, float &roll);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _FUSION_UTIL_H_ */
