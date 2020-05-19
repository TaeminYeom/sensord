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

#include "message.h"

#include <sensor_log.h>
#include <atomic>

using namespace ipc;

#define UNDEFINED_TYPE -2

static std::atomic<uint64_t> sequence(0);

message::message(size_t capacity)
	: m_size(0)
	, m_capacity(capacity)
	, m_msg((char *)malloc(sizeof(char) * capacity))
{
	m_header.id = sequence++;
	m_header.type = UNDEFINED_TYPE;
	m_header.length = m_size;
	m_header.err = 0;
}

message::message(const void *msg, size_t sz)
	: m_size(sz)
	, m_capacity(sz)
	, m_msg((char *)msg)
{
	m_header.id = sequence++;
	m_header.type = UNDEFINED_TYPE;
	m_header.length = m_size;
	m_header.err = 0;
}

message::message(const message &msg)
	: m_size(msg.m_size)
	, m_capacity(msg.m_capacity)
	, m_msg((char *)malloc(sizeof(char) * msg.m_capacity))
{
	::memcpy(&m_header, &msg.m_header, sizeof(message_header));
	::memcpy(m_msg, msg.m_msg, msg.m_size);
}

message::message(int error)
	: m_size(0)
	, m_capacity(0)
	, m_msg(NULL)
{
	m_header.id = sequence++;
	m_header.type = UNDEFINED_TYPE;
	m_header.length = 0;
	m_header.err = error;
}

message::~message()
{
	if (m_msg) {
		free(m_msg);
		m_msg = NULL;
	}
}

void message::enclose(const void *msg, const size_t sz)
{
	if (!msg || sz == 0)
		return;

	if (m_capacity < sz)
		return;

	::memcpy(reinterpret_cast<char *>(m_msg), msg, sz);
	m_size = sz;
	m_header.length = sz;
}

void message::enclose(int error)
{
	m_header.err = error;
	m_header.length = 0;
	m_size = 0;
}

void message::disclose(void *msg, const size_t size)
{
	if (!msg || !m_msg || m_size > size)
		return;

	::memcpy(msg, m_msg, m_size);
}

uint32_t message::type(void)
{
	return m_header.type;
}

void message::set_type(uint32_t msg_type)
{
	m_header.type = msg_type;
}

size_t message::size(void)
{
	return m_size;
}

message_header *message::header(void)
{
	return &m_header;
}

char *message::body(void)
{
	return m_msg;
}
