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

#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__

#include <stdint.h>
#include <glib.h>
#include <atomic>
#include <map>

#include "event_handler.h"
#include "cmutex.h"

namespace ipc {

enum event_condition_e {
	EVENT_IN =  G_IO_IN,
	EVENT_OUT = G_IO_OUT,
	EVENT_HUP = G_IO_HUP,
	EVENT_NVAL = G_IO_NVAL,
};

/* move it to file */
class idle_handler {
	virtual ~idle_handler();
	bool handle(int fd);
};

class event_loop;

class handler_info {
public:
	handler_info(uint64_t _id, int _fd, GIOChannel *_ch, GSource *_src, event_handler *_handler, event_loop *_loop)
	: id(_id)
	, fd(_fd)
	, g_ch(_ch)
	, g_src(_src)
	, handler(_handler)
	, loop(_loop)
	{}

	uint64_t id;
	int fd;
	GIOChannel *g_ch;
	GSource *g_src;
	event_handler *handler;
	event_loop *loop;
};

class event_loop {
public:
	typedef unsigned int event_condition;
	typedef bool (*idle_cb)(void *);

	event_loop();
	event_loop(GMainLoop *mainloop);
	~event_loop();

	void set_mainloop(GMainLoop *mainloop);

	uint64_t add_event(const int fd, const event_condition cond, event_handler *handler);
	size_t add_idle_event(unsigned int priority, void (*fn)(size_t, void*), void* data);

	bool remove_event(uint64_t id, bool close_channel = false);
	void remove_all_events(void);

	void release_info(handler_info *info);

	bool run(int timeout = 0);
	void stop(void);
	void terminate(void);

	bool is_running(void);
	bool is_terminator(int fd);

private:
	GMainLoop *m_mainloop;
	std::atomic<bool> m_running;
	std::atomic<bool> m_terminating;
	std::atomic<uint64_t> m_sequence;
	std::map<uint64_t, handler_info *> m_handlers;

	int m_term_fd;
	cmutex m_cmutex;
};

}

#endif /* __EVENT_LOOP_H__ */
