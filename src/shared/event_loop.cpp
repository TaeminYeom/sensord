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

#include "event_loop.h"

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <sys/eventfd.h>
#include <glib.h>

#include <vector>
#include <queue>

#include "channel_event_handler.h"
#include "sensor_log.h"
#include "event_handler.h"
#include "channel.h"

#define BAD_HANDLE 0

using namespace ipc;
using namespace sensor;

static std::vector<channel_handler*> channel_handler_release_list;
static std::priority_queue<channel*> channel_release_queue;
static sensor::cmutex release_lock;

static void release_res()
{
	AUTOLOCK(release_lock);

	channel *prev = NULL, *current = NULL;
	while (!channel_release_queue.empty()) {
		current = channel_release_queue.top();
		if (prev != current)
			delete current;
		prev = current;
		channel_release_queue.pop();
	}

	for (auto &it : channel_handler_release_list)
		delete it;

	/* To reduce memory allocation, swap to new data structure.
	   This prevents occasional over-allocation of memory. */
	std::priority_queue<channel*>().swap(channel_release_queue);
	std::vector<channel_handler*>().swap(channel_handler_release_list);
}

static gboolean g_io_handler(GIOChannel *ch, GIOCondition condition, gpointer data)
{
	uint64_t id;
	int fd;
	bool term;
	bool ret;
	event_loop *loop;
	event_handler *handler;
	unsigned int cond;

	cond = (unsigned int)condition;

	if (cond & (G_IO_HUP))
		cond &= ~(G_IO_IN | G_IO_OUT);

	handler_info *info = (handler_info *)data;
	loop = info->loop;
	handler = info->handler;
	retvm_if(!loop || !handler, FALSE, "Invalid event info");

	id = info->id;
	fd = info->fd;
	term = loop->is_terminator(fd);

	if (cond & G_IO_NVAL)
		return G_SOURCE_REMOVE;

	void *addr = NULL;
	ret = handler->handle(fd, (event_condition)cond, &addr);

	if (!ret && !term) {
		LOCK(release_lock);
		channel_release_queue.push((channel*)addr);
		UNLOCK(release_lock);
		if (!addr)
			loop->remove_event(id);
		ret = G_SOURCE_REMOVE;
	} else {
		ret = G_SOURCE_CONTINUE;
	}

	release_res();

	return ret;
}

static gint on_timer(gpointer data)
{
	event_loop *loop = (event_loop *)data;
	loop->stop();

	return FALSE;
}

event_loop::event_loop()
: m_mainloop(NULL)
, m_running(false)
, m_terminating(false)
, m_sequence(1)
, m_term_fd(-1)
{
	m_mainloop = g_main_loop_new(NULL, FALSE);
}

event_loop::event_loop(GMainLoop *mainloop)
: m_mainloop(NULL)
, m_running(true)
, m_terminating(false)
, m_sequence(1)
, m_term_fd(-1)
{
	m_mainloop = mainloop;
}

event_loop::~event_loop()
{
	if (m_term_fd != -1)
		close(m_term_fd);

	_D("Destoryed");
}

void event_loop::set_mainloop(GMainLoop *mainloop)
{
	retm_if(!mainloop, "Invalid mainloop");

	m_mainloop = mainloop;
}

uint64_t event_loop::add_event(const int fd, const event_condition cond, event_handler *handler)
{
	AUTOLOCK(m_cmutex);
	GIOChannel *ch = NULL;
	GSource *src = NULL;

	retvm_if(m_terminating.load(), BAD_HANDLE,
			"Failed to add event, because event_loop is being terminated");

	ch = g_io_channel_unix_new(fd);
	retvm_if(!ch, BAD_HANDLE, "Failed to create g_io_channel_unix_new");

	src = g_io_create_watch(ch, (GIOCondition)(cond));
	if (!src) {
		g_io_channel_unref(ch);
		_E("Failed to create g_io_create_watch");
		return BAD_HANDLE;
	}

	uint64_t id = m_sequence++;
	if (m_sequence == 0) {
		m_sequence = 1;
	}

	handler_info *info = new(std::nothrow) handler_info(id, fd, ch, src, handler, this);
	retvm_if(!info, BAD_HANDLE, "Failed to allocate memory");

	handler->set_event_id(id);
	g_source_set_callback(src, (GSourceFunc) g_io_handler, info, NULL);
	g_source_attach(src, g_main_loop_get_context(m_mainloop));

	m_handlers[id] = info;

	/* _D("Added event[%llu], fd[%d]", id, fd); */
	return id;
}

struct idler_data {
	void (*m_fn)(size_t, void*);
	void* m_data;
};

size_t event_loop::add_idle_event(unsigned int priority, void (*fn)(size_t, void*), void* data)
{
	AUTOLOCK(m_cmutex);
	GSource *src;

	retvm_if(m_terminating.load(), 0,
			"Failed to remove event, because event_loop is terminated");

	src = g_idle_source_new();
	retvm_if(!src, 0, "Failed to allocate memory");

	idler_data *id = new idler_data();
	id->m_fn = fn;
	id->m_data = data;

	g_source_set_callback(src, [](gpointer gdata) -> gboolean {
		idler_data *id = (idler_data *)gdata;
		id->m_fn((size_t)id, id->m_data);
		delete id;
		return G_SOURCE_REMOVE;
	}, id, NULL);

	g_source_attach(src, g_main_loop_get_context (m_mainloop));
	g_source_unref(src);

	return (size_t)id;
}

bool event_loop::remove_event(uint64_t id)
{
	AUTOLOCK(m_cmutex);
	auto it = m_handlers.find(id);
	retv_if(it == m_handlers.end(), false);

	release_info(it->second);
	m_handlers.erase(id);

	/* _D("Removed event[%llu]", id); */
	return true;
}

void event_loop::remove_all_events(void)
{
	AUTOLOCK(m_cmutex);
	auto it = m_handlers.begin();
	while (it != m_handlers.end()) {
		release_info(it->second);
		it = m_handlers.erase(it);
	}
}

void event_loop::release_info(handler_info *info)
{
	retm_if(!info->g_ch || info->id == 0, "Invalid handler information");
	/* _D("Releasing event..[%llu]", info->id); */

	g_source_destroy(info->g_src);
	g_source_unref(info->g_src);

	g_io_channel_unref(info->g_ch);

	info->g_ch = NULL;
	delete info->handler;
	info->handler = NULL;

	delete info;

	/* _D("Released event[%llu]", info->id); */
}

void event_loop::add_channel_release_queue(channel *ch)
{
	AUTOLOCK(release_lock);
	channel_release_queue.push(ch);
}

void event_loop::add_channel_handler_release_list(channel_handler *handler)
{
	AUTOLOCK(release_lock);
	channel_handler_release_list.push_back(handler);
}

class terminator : public event_handler
{
public:
	terminator(event_loop *loop)
	: m_loop(loop)
	{ }

	bool handle(int fd, event_condition condition, void **data)
	{
		m_loop->terminate();
		return false;
	}

private:
	event_loop *m_loop;
};

bool event_loop::run(int timeout)
{
	retvm_if(!m_mainloop, false, "Invalid GMainLoop");
	retvm_if(is_running(), false, "Already started");

	if (timeout > 0) {
		GSource *src = g_timeout_source_new(timeout);
		g_source_set_callback(src, on_timer, this, NULL);
		g_source_attach(src, g_main_loop_get_context(m_mainloop));
		g_source_unref(src);
	}

	m_term_fd = eventfd(0, EFD_CLOEXEC);
	retv_if(m_term_fd == -1, false);

	terminator *handler = new(std::nothrow) terminator(this);
	retvm_if(!handler, false, "Failed to allocate memory");

	add_event(m_term_fd, EVENT_IN | EVENT_HUP | EVENT_NVAL, handler);

	m_running.store(true);

	_I("Started");
	g_main_loop_run(m_mainloop);

	return true;
}

void event_loop::stop(void)
{
	ret_if(!is_running() || m_terminating.load());

	terminate();
}

void event_loop::terminate(void)
{
	remove_all_events();

	if (m_mainloop) {
		g_main_loop_quit(m_mainloop);
		g_main_loop_unref(m_mainloop);
		m_mainloop = NULL;
	}

	m_running.store(false);
	m_terminating.store(false);

	_I("Terminated");
}

bool event_loop::is_running(void)
{
	return m_running.load();
}

bool event_loop::is_terminator(int fd)
{
	return (m_term_fd == fd);
}
