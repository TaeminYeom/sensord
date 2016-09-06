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
#include <cmath>
#include <climits>
#include <sensor_log.h>
#include <face_down_alg_impl.h>

#define GRAVITY 9.80665
#define TWENTY_DEGREES 0.349066
#define ONE_SIXTY_DEGREES 2.79253
#define WINDOW_SIZE (2000*100)

face_down_alg_impl::face_down_alg_impl()
{
	m_current_time = 0;
	m_last_event_time = 0;
	m_latest_down_time = 0;
}

face_down_alg_impl::~face_down_alg_impl()
{
}

void face_down_alg_impl::push_event(const sensor_event_t & event)
{
	//_I("face_down_alg: %llu acc[2]: %f",event.data->timestamp,event.data->values[2]);
	m_current_time = event.data->timestamp;
	remove_old_up_time();

	if (event.data->values[2] < (GRAVITY * cos(ONE_SIXTY_DEGREES)))
		m_latest_down_time = event.data->timestamp;

	if (event.data->values[2] > (GRAVITY * cos(TWENTY_DEGREES)))
		m_oldest_up_time.push(event.data->timestamp);
}

void face_down_alg_impl::remove_old_up_time(void)
{
	while (m_oldest_up_time.size() > 0 && (m_current_time - m_oldest_up_time.front() > WINDOW_SIZE))
		m_oldest_up_time.pop();
}

bool face_down_alg_impl::get_face_down(void)
{
	unsigned long long down = is_facing_down();
	unsigned long long up = was_facing_up();
	//_I("face_down_alg: down: %llu, up: %llu", down, up);

	if (down < up)
		return false;

	if (m_current_time - m_last_event_time < WINDOW_SIZE)
		return false;

	m_last_event_time = m_current_time;
	return true;
}

unsigned long long face_down_alg_impl::is_facing_down(void)
{
	if (m_current_time - m_latest_down_time < WINDOW_SIZE)
		return m_latest_down_time;
	return 0;
}

unsigned long long face_down_alg_impl::was_facing_up(void)
{
	remove_old_up_time();
	if (m_oldest_up_time.size() == 0)
		return ULLONG_MAX;
	return m_oldest_up_time.front();
}
