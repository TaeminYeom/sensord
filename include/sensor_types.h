/*
 * sensord
 *
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

#ifndef __SENSOR_TYPES_H__
#define __SENSOR_TYPES_H__

/* TODO: It is for compatibility with capi-system-sensor */
#define __SENSOR_COMMON_H__

#include <stddef.h>
#include <stdint.h>
#include <sensor_hal_types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef OP_SUCCESS
#define OP_SUCCESS 0
#endif
#ifndef OP_ERROR
#define OP_ERROR   -1
#endif
#ifndef OP_DEFAULT
#define OP_DEFAULT 1
#endif
#ifndef OP_CONTINUE
#define OP_CONTINUE 2
#endif
#ifndef NAME_MAX
#define NAME_MAX 256
#endif

#define SENSOR_TYPE_SHIFT 32
#define SENSOR_EVENT_SHIFT 16
#define SENSOR_INDEX_MASK 0xFFFFFFFF

#define CONVERT_ID_TYPE(id) ((id) >> SENSOR_TYPE_SHIFT)
#define CONVERT_TYPE_EVENT(type) ((type) << SENSOR_EVENT_SHIFT | 0x1)

#define MICROSECONDS(tv)        ((tv.tv_sec * 1000000ll) + tv.tv_usec)

#define SENSOR_UNKNOWN_TYPE "http://tizen.org/sensor/unknown"
#define SENSOR_UNKNOWN_NAME "Unknown"

#define SENSOR_URI_PERMISSION_DENIED "http://tizen.org/sensor/EACCES"

typedef int64_t sensor_id_t;
typedef void *sensor_t;

typedef enum sensor_type_t {
	UNKNOWN_SENSOR = -2,
	ALL_SENSOR = -1,
	ACCELEROMETER_SENSOR = 0,
	GRAVITY_SENSOR,
	LINEAR_ACCEL_SENSOR,
	GEOMAGNETIC_SENSOR,
	ROTATION_VECTOR_SENSOR,
	ORIENTATION_SENSOR,
	GYROSCOPE_SENSOR,
	LIGHT_SENSOR,
	PROXIMITY_SENSOR,
	PRESSURE_SENSOR,
	ULTRAVIOLET_SENSOR,
	TEMPERATURE_SENSOR,
	HUMIDITY_SENSOR,
	HRM_SENSOR,
	BIO_HRM_SENSOR = HRM_SENSOR,
	HRM_LED_GREEN_SENSOR,
	BIO_LED_GREEN_SENSOR = HRM_LED_GREEN_SENSOR,
	HRM_LED_IR_SENSOR,
	BIO_LED_IR_SENSOR = HRM_LED_IR_SENSOR,
	HRM_LED_RED_SENSOR,
	BIO_LED_RED_SENSOR = HRM_LED_RED_SENSOR,
	GYROSCOPE_UNCAL_SENSOR,
	GEOMAGNETIC_UNCAL_SENSOR,
	GYROSCOPE_RV_SENSOR,
	GEOMAGNETIC_RV_SENSOR,

	HUMAN_PEDOMETER_SENSOR = 0x300,
	HUMAN_SLEEP_MONITOR_SENSOR,
	HUMAN_SLEEP_DETECTOR_SENSOR,
	SLEEP_DETECTOR_SENSOR = HUMAN_SLEEP_DETECTOR_SENSOR,
	HUMAN_STRESS_MONITOR_SENSOR,
	STRESS_MONITOR_SENSOR = HUMAN_STRESS_MONITOR_SENSOR,

	EXERCISE_WALKING_SENSOR = 0x400,
	EXERCISE_RUNNING_SENSOR,
	EXERCISE_HIKING_SENSOR,
	EXERCISE_CYCLING_SENSOR,
	EXERCISE_ELLIPTICAL_SENSOR,
	EXERCISE_INDOOR_CYCLING_SENSOR,
	EXERCISE_ROWING_SENSOR,
	EXERCISE_STEPPER_SENSOR,

	EXTERNAL_EXERCISE_SENSOR = 0x800,
	EXERCISE_SENSOR = EXTERNAL_EXERCISE_SENSOR,

	FUSION_SENSOR = 0x900,
	AUTO_ROTATION_SENSOR,
	AUTO_BRIGHTNESS_SENSOR,

	GESTURE_MOVEMENT_SENSOR = 0x1200,
	GESTURE_WRIST_UP_SENSOR,
	GESTURE_WRIST_DOWN_SENSOR,
	GESTURE_MOVEMENT_STATE_SENSOR,
	GESTURE_FACE_DOWN_SENSOR,

	ACTIVITY_TRACKER_SENSOR = 0x1A00,
	ACTIVITY_LEVEL_MONITOR_SENSOR,
	GPS_BATCH_SENSOR,

	HRM_CTRL_SENSOR = 0x1A80,

	WEAR_STATUS_SENSOR = 0x2000,
	WEAR_ON_MONITOR_SENSOR,
	NO_MOVE_DETECTOR_SENSOR,
	RESTING_HR_SENSOR,
	STEP_LEVEL_MONITOR_SENSOR,
	EXERCISE_STANDALONE_SENSOR,
	EXERCISE_COACH_SENSOR = EXERCISE_STANDALONE_SENSOR,
	EXERCISE_HR_SENSOR,
	WORKOUT_SENSOR,
	AUTOSESSION_EXERCISE_SENSOR = WORKOUT_SENSOR,
	CYCLE_MONITOR_SENSOR,
	STAIR_TRACKER_SENSOR,
	PRESSURE_INDICATOR_SENSOR,
	PRESSURE_ALERT_SENSOR,
	HR_CALORIE_SENSOR,

	CONTEXT_SENSOR = 0x7000,
	MOTION_SENSOR,
	PIR_SENSOR,
	PIR_LONG_SENSOR,
	DUST_SENSOR,
	THERMOMETER_SENSOR,
	PEDOMETER_SENSOR,
	FLAT_SENSOR,
	HRM_RAW_SENSOR,
	BIO_SENSOR = HRM_RAW_SENSOR,
	TILT_SENSOR,
	RV_RAW_SENSOR,
	GSR_SENSOR,
	SIMSENSE_SENSOR,
	PPG_SENSOR,

	CUSTOM_SENSOR = 0X9000,
} sensor_type_t;

typedef struct sensor_info2_t {
	uint32_t id;
	sensor_type_t type;
	const char *uri;
	const char *vendor;
	float min_range;
	float max_range;
	float resolution;
	int min_interval;
	int max_batch_count;
	bool wakeup_supported;
	const char *privilege;
	void *reserved[8];
} sensor_info2_t;

typedef enum sensor_permission_t {
	SENSOR_PERMISSION_NONE = 0,
	SENSOR_PERMISSION_STANDARD = 1,
	SENSOR_PERMISSION_HEALTH_INFO = 2,
} sensor_permission_t;

typedef enum sensor_privilege_t {
	SENSOR_PRIVILEGE_PUBLIC = 1,
} sensor_privilege_t;

typedef struct sensor_event_t {
	unsigned int event_type;
	sensor_id_t sensor_id;
	unsigned int data_length;
	sensor_data_t *data;
} sensor_event_t;

/*
 *	To prevent naming confliction as using same enums as sensor CAPI use
 */
#ifndef __SENSOR_H__
enum sensor_option_t {
	SENSOR_OPTION_DEFAULT = 0,
	SENSOR_OPTION_ON_IN_SCREEN_OFF = 1,
	SENSOR_OPTION_ON_IN_POWERSAVE_MODE = 2,
	SENSOR_OPTION_ALWAYS_ON = SENSOR_OPTION_ON_IN_SCREEN_OFF | SENSOR_OPTION_ON_IN_POWERSAVE_MODE,
	SENSOR_OPTION_END
};

typedef enum sensor_option_t sensor_option_e;
#endif

enum sensord_attribute_e {
	SENSORD_ATTRIBUTE_AXIS_ORIENTATION = 1,
	SENSORD_ATTRIBUTE_PAUSE_POLICY,
	SENSORD_ATTRIBUTE_INTERVAL = 0x10,
	SENSORD_ATTRIBUTE_MAX_BATCH_LATENCY,
	SENSORD_ATTRIBUTE_PASSIVE_MODE,
	SENSORD_ATTRIBUTE_FLUSH,
};

enum sensord_axis_e {
	SENSORD_AXIS_DEVICE_ORIENTED = 1,
	SENSORD_AXIS_DISPLAY_ORIENTED,
};

enum sensord_pause_e {
	SENSORD_PAUSE_NONE = 0,
	SENSORD_PAUSE_ON_DISPLAY_OFF = 1,
	SENSORD_PAUSE_ON_POWERSAVE_MODE = 2,
	SENSORD_PAUSE_ALL = 3,
	SENSORD_PAUSE_END,
};

enum poll_interval_t {
	POLL_100HZ_MS	= 10,
	POLL_50HZ_MS	= 20,
	POLL_25HZ_MS	= 40,
	POLL_20HZ_MS	= 50,
	POLL_10HZ_MS	= 100,
	POLL_5HZ_MS		= 200,
	POLL_1HZ_MS		= 1000,
	POLL_MAX_HZ_MS  = 255000,
};

#define DEFAULT_INTERVAL POLL_10HZ_MS

enum sensor_interval_t {
	SENSOR_INTERVAL_FASTEST = POLL_100HZ_MS,
	SENSOR_INTERVAL_NORMAL = POLL_5HZ_MS,
};

enum proxi_change_state {
	PROXIMITY_STATE_NEAR = 0,
	PROXIMITY_STATE_FAR = 1,
};

enum auto_rotation_state {
	AUTO_ROTATION_DEGREE_UNKNOWN = 0,
	AUTO_ROTATION_DEGREE_0,
	AUTO_ROTATION_DEGREE_90,
	AUTO_ROTATION_DEGREE_180,
	AUTO_ROTATION_DEGREE_270,
};

#ifdef __cplusplus
}
#endif

#include <sensor_deprecated.h>

#endif /* __SENSOR_TYPES_H__ */
