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

#ifndef _PHYSICAL_SENSOR_H_
#define _PHYSICAL_SENSOR_H_

#include <sensor_base.h>
#include <worker_thread.h>
#include <sensor_hal.h>

class physical_sensor : public sensor_base {
public:
	physical_sensor();
	virtual ~physical_sensor();

	/* setting module */
	void set_sensor_info(const sensor_info_t *info);
	void set_sensor_device(sensor_device *device);

	/* module info */
	virtual sensor_type_t get_type(void);
	virtual unsigned int get_event_type(void);
	virtual const char* get_name(void);
	virtual uint32_t get_hal_id(void);

	int get_poll_fd(void);

	virtual bool on_event(const sensor_data_t *data, int data_len, int remains);

	virtual bool read_fd(std::vector<uint32_t> &ids);
	virtual int get_data(sensor_data_t **data, int *length);
	virtual bool flush(void);

protected:
	const sensor_info_t *m_info;
	sensor_device *m_sensor_device;
	uint32_t hal_id;

	virtual bool on_start(void);
	virtual bool on_stop(void);
	virtual int set_attribute(int32_t attribute, int32_t value);
	virtual int set_attribute(int32_t attribute, char *value, int len);
	virtual bool set_interval(unsigned long interval);
	virtual bool set_batch_latency(unsigned long latency);
	virtual bool get_sensor_info(sensor_info &info);

private:
	static cmutex m_mutex;
};

#endif /* _PHYSICAL_SENSOR_H_ */
