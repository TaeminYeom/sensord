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

#include "socket.h"

#include <fcntl.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <systemd/sd-daemon.h>

#include "sensor_log.h"

using namespace ipc;

static bool set_close_on_exec(int fd)
{
	if (::fcntl(fd, F_SETFL, FD_CLOEXEC) == -1)
		return false;

	return true;
}

static int create_systemd_socket(const std::string &path, int type)
{
	int n;
	int listening;

	listening = (type == SOCK_STREAM) ? 1 : -1;

	n = sd_listen_fds(0);
	retvm_if(n < 0, -EPERM, "Failed to listen fds from systemd");

	for (int fd = SD_LISTEN_FDS_START; fd < SD_LISTEN_FDS_START + n; ++fd) {
		if (sd_is_socket_unix(fd, type, listening, path.c_str(), 0) > 0) {
			set_close_on_exec(fd);
			return fd;
		}
	}

	return -EPERM;
}

static int create_unix_socket(int type)
{
	int sock_fd = ::socket(AF_UNIX, type, 0);

	if (sock_fd < 0) {
		_ERRNO(errno, _E, "Failed to create socket");
		return -EPERM;
	}

	set_close_on_exec(sock_fd);

	int optval = 1;
	if (::setsockopt(sock_fd, SOL_SOCKET, SO_PASSCRED, &optval, sizeof(optval)) < 0) {
		_ERRNO(errno, _E, "Failed to create socket[%d]", sock_fd);
		::close(sock_fd);
		return -EPERM;
	}

	return sock_fd;
}

static bool select_fds(int fd, fd_set *read_fds, fd_set *write_fds, const int timeout)
{
	struct timeval tv;
	int err;

	tv.tv_sec = timeout;
	tv.tv_usec = 0;

	while (true) {
		err = ::select(fd + 1, read_fds, write_fds, NULL, &tv);
		if (err <= 0)
			return false;

		if (read_fds && FD_ISSET(fd, read_fds))
			break;
		if (write_fds && FD_ISSET(fd, write_fds))
			break;
	}

	return true;
}

socket::socket()
: m_sock_fd(-1)
, m_mode(MSG_DONTWAIT | MSG_NOSIGNAL)
, m_listening(false)
{
}

socket::socket(int sock_fd)
: m_sock_fd(sock_fd)
, m_mode(MSG_DONTWAIT | MSG_NOSIGNAL)
, m_listening(false)
{
}

socket::socket(const socket &sock)
: m_sock_fd(-1)
, m_mode(MSG_DONTWAIT | MSG_NOSIGNAL)
, m_listening(false)
{
	if (this == &sock)
		return;

	m_sock_fd = sock.m_sock_fd;
	m_mode = sock.m_mode;
	m_listening.store(sock.m_listening);
}

socket::~socket()
{
	close();
}

bool socket::connect(void)
{
	const int TIMEOUT = 3;
	sockaddr_un addr;
	fd_set write_fds;
	FD_ZERO(&write_fds);
	FD_SET(m_sock_fd, &write_fds);

	retvm_if(m_path.size() >= sizeof(sockaddr_un::sun_path), false,
			"Failed to create socket[%s]", m_path.c_str());

	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, m_path.c_str(), sizeof(sockaddr_un::sun_path));
	addr.sun_path[m_path.size()] = '\0';

	if (::connect(m_sock_fd,
				reinterpret_cast<struct sockaddr *>(&addr),
				sizeof(struct sockaddr_un)) < 0) {
		_ERRNO(errno, _E, "Failed to connect() for socket[%d]", m_sock_fd);
		close();
		return false;
	}

	if (!select_fds(m_sock_fd, NULL, &write_fds, TIMEOUT)) {
		_E("Failed to select for socket[%d]", m_sock_fd);
		close();
		return false;
	}

	if (!has_connected()) {
		close();
		return false;
	}

	_D("Connected[%d]", m_sock_fd);

	return true;
}

bool socket::bind(void)
{
	sockaddr_un addr;
	int file_mode;

	retvm_if(m_path.size() >= sizeof(sockaddr_un::sun_path), false,
			"Failed to create socket[%s]", m_path.c_str());
	retv_if(m_listening.load(), true);

	if (!access(m_path.c_str(), F_OK))
		unlink(m_path.c_str());

	addr.sun_family = AF_UNIX;
	::strncpy(addr.sun_path, m_path.c_str(), sizeof(sockaddr_un::sun_path));
	addr.sun_path[m_path.size()] = '\0';

	if (::bind(m_sock_fd,
				reinterpret_cast<struct sockaddr *>(&addr),
				sizeof(struct sockaddr_un)) < 0) {
		_ERRNO(errno, _E, "Failed to bind for socket[%d]", m_sock_fd);
		close();
		return false;
	}

	/* TODO: Is this really necessary? */
	file_mode = (S_IRWXU | S_IRWXG | S_IRWXO);
	if (chmod(m_path.c_str(), file_mode) < 0) {
		_ERRNO(errno, _E, "Failed to create socket[%d]", m_sock_fd);
		close();
		return false;
	}

	_D("Bound to path[%d, %s]", m_sock_fd, m_path.c_str());

	return true;
}

bool socket::listen(const int max_connections)
{
	retv_if(m_listening.load(), true);

	if (::listen(m_sock_fd, max_connections) < 0) {
		_ERRNO(errno, _E, "Failed to listen() for socket[%d]", m_sock_fd);
		close();
		return false;
	}

	m_listening.store(true);

	_D("Listened[%d]", m_sock_fd);

	return true;
}

bool socket::accept(socket &client_sock)
{
	int fd;
	fd_set read_fds;
	FD_ZERO(&read_fds);
	FD_SET(m_sock_fd, &read_fds);

	fd = ::accept(m_sock_fd, NULL, NULL);

	if (fd < 0) {
		_ERRNO(errno, _E, "Failed to accept[%d]", m_sock_fd);
		return false;
	}

	set_close_on_exec(fd);
	client_sock.set_fd(fd);
	/* TODO : socket type should be adjusted here */

	_D("Accepted[%d, %d]", m_sock_fd, fd);

	return true;
}

bool socket::close(void)
{
	retv_if(m_sock_fd < 0, false);

	if (::close(m_sock_fd) < 0) {
		_ERRNO(errno, _E, "Failed to close socket[%d]", m_sock_fd);
		return false;
	}

	_D("Closed[%d]", m_sock_fd);

	m_sock_fd = -1;
	m_listening.store(false);

	return true;
}

int socket::get_fd(void) const
{
	return m_sock_fd;
}

void socket::set_fd(int sock_fd)
{
	m_sock_fd = sock_fd;
}

int socket::get_mode(void) const
{
	return m_mode;
}

bool socket::set_mode(int mode)
{
	/* TODO : implement send/recv message mode */
	return true;
}

bool socket::create(const std::string &path)
{
	return false;
}

ssize_t socket::send(const void *buffer, size_t size, bool select) const
{
	if (select) {
		const int TIMEOUT = 1;
		fd_set write_fds;
		FD_ZERO(&write_fds);
		FD_SET(m_sock_fd, &write_fds);

		if (!select_fds(m_sock_fd, NULL, &write_fds, TIMEOUT)) {
			_E("Failed to send message(timeout)");
			return 0;
		}
	}

	return on_send(buffer, size);
}

ssize_t socket::recv(void* buffer, size_t size, bool select) const
{
	/* WARNING: if select() is called here, it affects performance */
	return on_recv(buffer, size);
}

bool socket::create_by_type(const std::string &path, int type)
{
	m_sock_fd = ::create_systemd_socket(path, type);
	if (m_sock_fd < 0) {
		_D("Creating the UDS instead of systemd socket..");
		m_sock_fd = create_unix_socket(type);
	} else {
		m_listening.store(true);
	}

	retvm_if((m_sock_fd < 0), false, "Failed to create socket");

	/* non-blocking mode */
	retvm_if(!set_blocking_mode(false), false, "Failed to set non-blocking mode");
	/* recv timeout */
	retvm_if(!set_recv_timeout(1), false, "Failed to set timeout");
	/* TODO */
	/*retvm_if(!set_reuse_addr(), false, "Failed to reuse address"); */

	_D("Created[%d]", m_sock_fd);

	m_path = path;

	return true;
}

int socket::get_sock_type(void)
{
	socklen_t opt_len;
	int sock_type;
	opt_len = sizeof(sock_type);

	retvm_if(m_sock_fd < 0, false, "Invalid socket[%d]", m_sock_fd);

	if (getsockopt(m_sock_fd, SOL_SOCKET, SO_TYPE, &sock_type, &opt_len) < 0) {
	   _ERRNO(errno, _E, "Failed to getsockopt from socket[%d]", m_sock_fd);
	   return false;
	}

	return sock_type;
}

bool socket::set_recv_timeout(int sec)
{
	struct timeval timeout = {sec, 0};

	retvm_if(m_sock_fd < 0, false, "Invalid socket[%d]", m_sock_fd);

	if (setsockopt(m_sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
	   _ERRNO(errno, _E, "Failed to setsockopt[%d]", m_sock_fd);
	   return false;
	}

	return true;
}

bool socket::set_sock_type(int type)
{
	socklen_t opt_len;
	opt_len = sizeof(type);

	retvm_if(m_sock_fd < 0, false, "Invalid socket[%d]", m_sock_fd);

	if (setsockopt(m_sock_fd, SOL_SOCKET, SO_TYPE, &type, opt_len) < 0) {
	   _ERRNO(errno, _E, "Failed to setsockopt[%d]", m_sock_fd);
	   return false;
	}

	return true;
}

bool socket::set_blocking_mode(bool blocking)
{
	int flags;

	flags = fcntl(m_sock_fd, F_GETFL);
	retvm_if(flags == -1, false, "Failed to fcntl(F_GETFL)[%d]", m_sock_fd);

	flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);

	flags = fcntl(m_sock_fd, F_SETFL, flags);
	retvm_if(flags == -1, false, "Failed to fcntl(F_SETFL)[%d]", m_sock_fd);

	return true;
}

bool socket::has_connected(void)
{
	int so_error;
	socklen_t len = sizeof(so_error);

	if (getsockopt(m_sock_fd, SOL_SOCKET, SO_ERROR, &so_error, &len) == -1) {
		_E("Failed to call getsockopt[%d]", m_sock_fd);
		return false;
	}

	if (so_error) {
		_E("Failed to connect[%d]: %d", so_error);
		return false;
	}

	return true;
}

bool socket::set_buffer_size(int type, int size)
{
	retv_if(m_sock_fd < 0, false);

	int ret = 0;

	ret = setsockopt(m_sock_fd, SOL_SOCKET, type, &size, sizeof(size));
	retvm_if(ret < 0, false, "Failed to call setsocketopt()");

	return true;
}

int socket::get_buffer_size(int type)
{
	retv_if(m_sock_fd < 0, false);

	int ret = 0;
	int buf_size = 0;
	socklen_t len;

	ret = getsockopt(m_sock_fd, SOL_SOCKET, type, &buf_size, &len);
	retvm_if(ret < 0, -EPERM, "Failed to call getsocketopt()");

	return buf_size;
}

int socket::get_current_buffer_size(void)
{
	retv_if(m_sock_fd < 0, false);

	int queue_size = 0;
	ioctl(m_sock_fd, TIOCOUTQ, &queue_size);

	return queue_size;
}

