/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#include <sensor_log.h>
#include <fusion_sensor.h>
#include <vector>

#ifdef ENABLE_AUTO_ROTATION
#include "auto_rotation/auto_rotation_sensor.h"
#endif
#ifdef ENABLE_GRAVITY
#include "gravity/gravity_lowpass_sensor.h"
#include "gravity/gravity_comp_sensor.h"
#endif
#ifdef ENABLE_LINEAR_ACCEL
#include "linear_accel/linear_accel_sensor.h"
#endif
#ifdef ENABLE_ROTATION_VECTOR
#include "rotation_vector/rv_sensor.h"
#include "rotation_vector/magnetic_rv_sensor.h"
#include "rotation_vector/gyro_rv_sensor.h"
#endif
#ifdef ENABLE_ORIENTATION
#include "orientation/orientation_sensor.h"
#include "orientation/magnetic_orientation_sensor.h"
#include "orientation/gyro_orientation_sensor.h"
#endif
#ifdef ENABLE_PEDOMETER
#include "pedometer/pedometer_sensor.h"
#endif
#ifdef ENABLE_GESTURE
#include "gesture/face_down_sensor.h"
#include "gesture/pick_up_sensor.h"
#endif

static std::vector<fusion_sensor_t> sensors;

template<typename _sensor>
void create_sensor(const char *name)
{
	fusion_sensor *instance = NULL;
	try {
		instance = new _sensor;
	} catch (std::exception &e) {
		_E("Failed to create %s sensor, exception: %s", name, e.what());
		return;
	} catch (int err) {
		_ERRNO(err, _E, "Failed to create %s sensor device", name);
		return;
	}

	sensors.push_back(instance);
}

extern "C" int create(fusion_sensor_t **fsensors)
{
#ifdef ENABLE_AUTO_ROTATION
	create_sensor<auto_rotation_sensor>("Auto Rotation Sensor");
#endif

#ifdef ENABLE_GRAVITY
	create_sensor<gravity_lowpass_sensor>("Gravity(Lowpass)");
	create_sensor<gravity_comp_sensor>("Gravity(Complementary)");
#endif

#ifdef ENABLE_LINEAR_ACCEL
	create_sensor<linear_accel_sensor>("Linear Acceleration Sensor");
#endif

#ifdef ENABLE_ROTATION_VECTOR
	create_sensor<rv_sensor>("Rotation Vector Sensor");
	create_sensor<magnetic_rv_sensor>("Magnetic Rotation Vector Sensor");
	create_sensor<gyro_rv_sensor>("Gyroscope Rotation Vector Sensor");
#endif

#ifdef ENABLE_ORIENTATION
	create_sensor<orientation_sensor>("Orientation Sensor");
	create_sensor<magnetic_orientation_sensor>("Magnetic Orientation Sensor");
	create_sensor<gyro_orientation_sensor>("Gyroscope Orientation Sensor");
#endif

#ifdef ENABLE_PEDOMETER
	create_sensor<pedometer_sensor>("Pedometer");
#endif

#ifdef ENABLE_GESTURE
	create_sensor<face_down_sensor>("Face Down Sensor");
	create_sensor<pick_up_sensor>("Pick Up Sensor");
#endif
	*fsensors = &sensors[0];
	return sensors.size();
}
