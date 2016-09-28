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

#ifndef __FACE_DOWN_ALG_IMPL_H__
#define __FACE_DOWN_ALG_IMPL_H__

#include <sensor_common.h>
#include <face_down_alg.h>
#include <queue>

class face_down_alg_impl : public virtual face_down_alg {
public:
	face_down_alg_impl();
	~face_down_alg_impl();
	void push_event(const sensor_event_t & event);
	bool get_face_down(void);
private:
	unsigned long long m_current_time;
	unsigned long long m_last_event_time;
	unsigned long long m_latest_down_time;
	std::queue<unsigned long long> m_oldest_up_time;
	void remove_old_up_time(void);
	unsigned long long is_facing_down();
	unsigned long long was_facing_up();

};

#endif /* __FACE_DOWN_ALG_IMPL_H__ */
