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

#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <atomic>

namespace ipc {

class socket {
public:
	socket();
	socket(int sock_fd);
	socket(const socket &sock);
	virtual ~socket();

	virtual bool create(const std::string &path);

	bool connect(void);
	bool bind(void);
	bool listen(const int max_connections);
	bool accept(socket &client_sock);

	bool close(void);

	int  get_fd(void) const;
	void set_fd(int sock_fd);

	int  get_mode(void) const;
	bool set_mode(int mode);

	bool set_blocking_mode(bool blocking);
	bool set_recv_timeout(int timeout);

	/* type : SO_SNDBUF, SO_RCVBUF */
	bool set_buffer_size(int type, int size);
	int  get_buffer_size(int type);
	int  get_current_buffer_size(void);

	ssize_t send(const void *buffer, size_t size, bool select = false) const;
	ssize_t recv(void* buffer, size_t size, bool select = false) const;

protected:
	bool create_by_type(const std::string &path, int type);

private:
	virtual ssize_t on_send(const void *buffer, size_t size) const = 0;
	virtual ssize_t on_recv(void* buffer, size_t size) const = 0;

	int  get_sock_type(void);
	bool set_sock_type(int type);
	bool has_connected(void);

	int m_sock_fd;
	int m_mode;
	std::atomic<bool> m_listening;
	std::string m_path;
};

}

#endif /* __SOCKET_H__ */
