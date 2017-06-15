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

#include "seqpacket_socket.h"

#include <sys/types.h>
#include <sys/socket.h>

#include "sensor_log.h"

using namespace ipc;

seqpacket_socket::seqpacket_socket()
: socket()
{
}

seqpacket_socket::~seqpacket_socket()
{
}

bool seqpacket_socket::create(const std::string &path)
{
	return socket::create_by_type(path, SOCK_SEQPACKET);
}

ssize_t seqpacket_socket::on_send(const void *buffer, size_t size) const
{
	ssize_t err, len;

	do {
		len = ::send(socket::get_fd(),
				reinterpret_cast<const char *>(buffer),
				size,
				socket::get_mode());

		err = len < 0 ? errno : 0;
	} while (err == EINTR);

	if (err) {
		_ERRNO(errno, _E, "Failed to send(%d, %p, %u) = %d",
			socket::get_fd(), buffer, size, len);
	}

	return err == 0 ? len : -err;
}

ssize_t seqpacket_socket::on_recv(void *buffer, size_t size) const
{
	ssize_t err, len;

	do {
		len = ::recv(socket::get_fd(),
				reinterpret_cast<char *>(buffer),
				size,
				socket::get_mode());

		if (len > 0) {
			err = 0;
		} else if (len == 0) {
			_E("Failed to recv(%d, %p , %u) = %d, because the peer performed shutdown!",
				socket::get_fd(), buffer, size, len);
			err = 1;
		} else {
			err = errno;
		}
	} while (err == EINTR);

	if ((err == EAGAIN) || (err == EWOULDBLOCK))
		return 0;

	if (err) {
		_ERRNO(errno, _E, "Failed to recv(%d, %p, %u) = %d",
			socket::get_fd(), buffer, size, len);
	}

	return err == 0 ? len : -err;
}

