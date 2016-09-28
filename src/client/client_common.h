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

#ifndef _CLIENT_COMMON_H_
#define _CLIENT_COMMON_H_

#include <sensor_internal.h>
#include <sensor_handle_info.h>
#include <reg_event_info.h>
#include <sensor_log.h>

#include <memory>

#define BASE_GATHERING_INTERVAL	100
#define THREAD_TERMINATION -1

typedef struct {
	unsigned long long event_id;
	int handle;
	sensor_t sensor;
	unsigned int event_type;
	sensor_cb_t cb;
	std::shared_ptr<void> sensor_data;
	void *user_data;
	sensor_accuracy_changed_cb_t accuracy_cb;
	unsigned long long timestamp;
	int accuracy;
	void *accuracy_user_data;
} client_callback_info;

const char *get_sensor_name(sensor_id_t sensor_id);
const char *get_event_name(unsigned int event_type);

unsigned int get_calibration_event_type(unsigned int event_type);
unsigned int get_log_per_count(sensor_id_t id);

void print_event_occurrence_log(sensor_handle_info &sensor_handle_info);

class sensor_info;
sensor_info *sensor_to_sensor_info(sensor_t sensor);
sensor_t sensor_info_to_sensor(const sensor_info *info);

#endif /* _CLIENT_COMMON_H_ */
