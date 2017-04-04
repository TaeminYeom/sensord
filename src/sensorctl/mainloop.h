/*
 * sensorctl
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

#pragma once /* __MAINLOOP_H__ */

#include <glib.h>
#include <gio/gio.h>
#include <atomic>

class mainloop {
public:
	static void run(void);
	static void stop(void);
	static bool is_running(void);

private:
	static mainloop& instance();

	void start_loop(void);
	void stop_loop(void);
	bool is_loop_running(void);

	GMainLoop *m_mainloop;
	std::atomic<bool> m_running;
};
