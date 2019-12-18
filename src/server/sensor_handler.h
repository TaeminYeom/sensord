/*
 * sensord
 *
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

#ifndef __SENSOR_HANDLER_H__
#define __SENSOR_HANDLER_H__

#include <sensor_observer.h>
#include <sensor_publisher.h>
#include <sensor_types.h>
#include <sensor_info.h>
#include <list>
#include <map>
#include <vector>

namespace sensor {

class sensor_handler : public sensor_publisher {
public:
	sensor_handler(const sensor_info &info);
	virtual ~sensor_handler() {}

	/* publisher */
	bool has_observer(sensor_observer *ob);
	bool add_observer(sensor_observer *ob);
	void remove_observer(sensor_observer *ob);
	int notify(const char *type, sensor_data_t *data, int len);
	uint32_t observer_count(void);

	virtual const sensor_info &get_sensor_info(void) = 0;

	virtual int start(sensor_observer *ob) = 0;
	virtual int stop(sensor_observer *ob) = 0;

	virtual int set_interval(sensor_observer *ob, int32_t interval) = 0;
	virtual int set_batch_latency(sensor_observer *ob, int32_t latency) = 0;
	virtual int delete_batch_latency(sensor_observer *ob);
	virtual int set_attribute(sensor_observer *ob, int32_t attr, int32_t value) = 0;
	virtual int get_attribute(int32_t attr, int32_t* value);
	void update_attribute(int32_t attr, int32_t value);
	virtual int set_attribute(sensor_observer *ob, int32_t attr, const char *value, int len) = 0;
	virtual int get_attribute(int32_t attr, char **value, int *len);
	void update_attribute(int32_t attr, const char *value, int len);
	virtual int flush(sensor_observer *ob) = 0;
	virtual int get_data(sensor_data_t **data, int *len) = 0;

	void set_cache(sensor_data_t *data, int size);
	int get_cache(sensor_data_t **data, int *len);
	bool notify_attribute_changed(uint32_t id, int attribute, int value);
	bool notify_attribute_changed(uint32_t id, int attribute, const char *value, int len);

protected:
	sensor_info m_info;
	std::map<int32_t, int32_t> m_attributes_int;
	std::map<int32_t, std::vector<char>> m_attributes_str;

private:
	std::list<sensor_observer *> m_observers;

	std::vector<char> m_sensor_data_cache;
};

}

#endif /* __SENSOR_HANDLER_H__ */
