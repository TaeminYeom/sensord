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
#ifndef __FUSION_BASE_H__
#define __FUSION_BASE_H__

#include "fusion.h"
#include "fusion_utils/orientation_filter.h"

class fusion_base : public virtual fusion {
public:
	fusion_base();
	virtual ~fusion_base();

	virtual void push_accel(sensor_data_t &data);
	virtual void push_gyro(sensor_data_t &data);
	virtual void push_mag(sensor_data_t &data);
	virtual bool get_rv(unsigned long long &timestamp, float &w, float &x, float &y, float &z);

protected:
	android::orientation_filter m_orientation_filter;

	bool m_enable_accel;
	bool m_enable_gyro;
	bool m_enable_magnetic;

	float m_x;
	float m_y;
	float m_z;
	float m_w;

	unsigned long long m_timestamp;
	unsigned long long m_timestamp_accel;
	unsigned long long m_timestamp_gyro;
	unsigned long long m_timestamp_mag;

	void clear();
	void store_orientation(void);
};



#endif /* __FUSION_BASE_H__ */
