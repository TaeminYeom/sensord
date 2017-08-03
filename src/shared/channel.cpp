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

#include "channel.h"

#include <stdint.h>
#include <unistd.h>
#include <memory>

#include "sensor_log.h"
#include "channel_event_handler.h"

#define SYSTEMD_SOCK_BUF_SIZE 40000

using namespace ipc;

class send_event_handler : public event_handler
{
public:
	send_event_handler(channel *ch, message *msg)
	: m_ch(ch)
	, m_msg(msg)
	{ }

	bool handle(int fd, event_condition condition)
	{
		if (!m_ch || !m_ch->is_connected())
			return false;

		if (condition & (EVENT_IN | EVENT_HUP))
			return false;

		if (!m_ch->send_sync(m_msg))
			return false;

		if (m_msg)
			m_msg->unref();

		return false;
	}

private:
	channel *m_ch;
	message *m_msg;
};

class read_event_handler : public event_handler
{
public:
	read_event_handler(channel *ch)
	: m_ch(ch)
	{ }

	bool handle(int fd, event_condition condition)
	{
		message msg;

		if (!m_ch || !m_ch->is_connected())
			return false;

		if (condition & (EVENT_OUT | EVENT_HUP))
			return false;

		if (!m_ch->read_sync(msg, false))
			return false;

		return false;
	}

private:
	channel *m_ch;
};

channel::channel(socket *sock)
: m_fd(sock->get_fd())
, m_event_id(0)
, m_socket(sock)
, m_handler(NULL)
, m_loop(NULL)
, m_connected(false)
{
}

channel::~channel()
{
	/* disconnect() should not be called here */
}

void channel::bind(channel_handler *handler, event_loop *loop)
{
	m_handler = handler;
	m_loop = loop;
	m_connected.store(true);

	if (m_handler)
		m_handler->connected(this);
}

void channel::bind(void)
{
	ret_if(!m_loop);
	m_event_id = m_loop->add_event(m_socket->get_fd(),
			(EVENT_IN | EVENT_HUP | EVENT_NVAL),
			dynamic_cast<channel_event_handler *>(m_handler));
}

bool channel::connect(channel_handler *handler, event_loop *loop)
{
	if (!m_socket->connect())
		return false;

	bind(handler, loop);
	return true;
}

void channel::disconnect(void)
{
	ret_if(!is_connected());
	m_connected.store(false);

	if (m_handler) {
		m_handler->disconnected(this);
		m_handler = NULL;
	}

	if (m_loop) {
		m_loop->remove_event(m_event_id, true);
		m_loop = NULL;
		m_event_id = 0;
	}

	if (m_socket) {
		delete m_socket;
		m_socket = NULL;
	}
}

bool channel::send(message *msg)
{
	retv_if(!m_loop, false);

	int cur_buffer_size = m_socket->get_current_buffer_size();
	retv_if(cur_buffer_size > SYSTEMD_SOCK_BUF_SIZE, false);

	send_event_handler *handler = new(std::nothrow) send_event_handler(this, msg);
	retvm_if(!handler, false, "Failed to allocate memory");

	msg->ref();

	m_loop->add_event(m_socket->get_fd(),
			(EVENT_OUT | EVENT_HUP | EVENT_NVAL) , handler);

	return true;
}

bool channel::send_sync(message *msg)
{
	retv_if(!msg, false);

	ssize_t size = 0;
	char *buf = msg->body();

	/* header */
	size = m_socket->send(reinterpret_cast<void *>(msg->header()),
				   sizeof(message_header), true);
	retv_if(size <= 0, false);
	retv_if(msg->size() <= 0, true);

	/* body */
	size = m_socket->send(buf, msg->size(), true);
	retv_if(size <= 0, false);

	return true;
}

bool channel::read(void)
{
	retv_if(!m_loop, false);

	read_event_handler *handler = new(std::nothrow) read_event_handler(this);
	retvm_if(!handler, false, "Failed to allocate memory");

	m_loop->add_event(m_socket->get_fd(), (EVENT_IN | EVENT_HUP | EVENT_NVAL), handler);

	return true;
}

bool channel::read_sync(message &msg, bool select)
{
	message_header header;
	ssize_t size = 0;
	char buf[MAX_MSG_CAPACITY];

	/* header */
	size = m_socket->recv(&header, sizeof(message_header), select);
	retv_if(size <= 0, false);

	/* check error from header */
	if (m_handler && header.err != 0) {
		m_handler->error_caught(this, header.err);
		msg.header()->err = header.err;
		return true;
	}

	/* body */
	if (header.length > 0) {
		size = m_socket->recv(&buf, header.length, select);
		retv_if(size <= 0, false);
	}

	buf[header.length] = '\0';
	msg.enclose(reinterpret_cast<const void *>(buf), header.length);
	msg.set_type(header.type);
	msg.header()->err = header.err;

	if (m_handler)
		m_handler->read(this, msg);

	return true;
}

bool channel::is_connected(void)
{
	return m_connected.load();
}

bool channel::set_option(int type, int value)
{
	switch (type) {
	case SO_SNDBUF:
		m_socket->set_buffer_size(type, value);
		break;
	case SO_RCVBUF:
		m_socket->set_buffer_size(type, value);
		break;
	default:
		break;
	}

	return true;
}

bool channel::get_option(int type, int &value) const
{
	switch (type) {
	case 0:
		value = m_socket->get_current_buffer_size();
		break;
	case SO_SNDBUF:
		value = m_socket->get_buffer_size(type);
		break;
	case SO_RCVBUF:
		value = m_socket->get_buffer_size(type);
		break;
	default:
		break;
	}

	return true;
}

int channel::get_fd(void) const
{
	return m_fd;
}

void channel::set_event_id(uint64_t id)
{
	m_event_id = id;
}
