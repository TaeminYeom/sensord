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

#ifndef _SENSOR_BASE_H_
#define _SENSOR_BASE_H_

#include <sensor_types.h>
#include <sensor_info_list.h>
#include <cmutex.h>

#include <sensor_log.h>
#include <sensor_common.h>
#include <worker_thread.h>
#include <sensor_info.h>
#include <sensor_hal_types.h>
#include <vector>

class sensor_base {
public:
	sensor_base();
	virtual ~sensor_base();

	/* id */
	void set_id(sensor_id_t id);
	sensor_id_t get_id(void);

	/* sensor info */
	virtual sensor_type_t get_type(void);
	virtual unsigned int get_event_type(void);
	virtual const char* get_name(void);
	virtual bool is_virtual(void);

	virtual bool get_sensor_info(sensor_info &info);

	/* set/get data */
	virtual int get_data(sensor_data_t **data, int *length);
	int get_cache(sensor_data_t **data);

	virtual bool flush(void);
	virtual int add_attribute(int client_id, int32_t attribute, int32_t value);
	virtual int add_attribute(int client_id, int32_t attribute, char *value, int value_size);
	virtual bool delete_attribute(int client_id);

	/* start/stop */
	bool start(void);
	bool stop(void);
	bool is_started(void);

	/* interval / batch */
	virtual bool add_interval(int client_id, unsigned int interval, bool is_processor);
	virtual bool delete_interval(int client_id, bool is_processor);
	unsigned int get_interval(int client_id, bool is_processor);

	virtual bool add_batch(int client_id, unsigned int latency);
	virtual bool delete_batch(int client_id);
	unsigned int get_batch(int client_id);

	bool push(sensor_event_t *event);

	/* permission(privilege) */
	int get_permission(void);

protected:
	sensor_data_t *m_last_data;

	void set_permission(int permission);

	unsigned long long get_timestamp(void);
	unsigned long long get_timestamp(timeval *t);

private:
	sensor_id_t m_id;
	int m_permission;

	sensor_info_list m_sensor_info_list;
	cmutex m_sensor_info_list_mutex;

	bool m_started;
	unsigned int m_client;
	cmutex m_client_mutex;

	cmutex m_data_cache_mutex;

	virtual int set_attribute(int32_t attribute, int32_t value);
	virtual int set_attribute(int32_t attribute, char *value, int value_size);

	virtual bool set_interval(unsigned long interval);
	virtual bool set_batch_latency(unsigned long latency);

	virtual bool pre_start(void);
	virtual bool on_start(void);
	virtual bool on_stop(void);

	void set_cache(sensor_data_t *data);
};

#endif /* _SENSOR_BASE_H_ */
