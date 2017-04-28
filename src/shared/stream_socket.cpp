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

#include "stream_socket.h"

#include <sys/types.h>
#include <sys/socket.h>

#include "sensor_log.h"

#define SLEEP_10_MS usleep(10000)

using namespace ipc;

stream_socket::stream_socket()
: socket()
{
}

stream_socket::~stream_socket()
{
}

bool stream_socket::create(const std::string &path)
{
	return socket::create_by_type(path, SOCK_STREAM);
}

ssize_t stream_socket::on_send(const void *buffer, size_t size) const
{
	ssize_t len = 0;
	size_t total_size = 0;

	do {
		len = ::send(get_fd(),
				reinterpret_cast<const char *>(buffer) + total_size,
				size - total_size, get_mode());

		if (len < 0) {
			if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK)) {
				SLEEP_10_MS;
				continue;
			}

			_ERRNO(errno, _E, "Failed to send(%d, %#p, %x, %d) = %d",
					get_fd(), buffer, total_size, size - total_size, len);
			return -errno;
		}

		total_size += len;
	} while (total_size < size);

	return total_size;
}

ssize_t stream_socket::on_recv(void *buffer, size_t size) const
{
	ssize_t len = 0;
	size_t total_size = 0;

	do {
		len = ::recv(get_fd(),
				reinterpret_cast<char *>(buffer) + total_size,
				size - total_size,
				socket::get_mode());

		if (len == 0) {
			_E("Failed to recv(%d, %#p + %x, %d) = %d, because the peer performed shutdown",
				get_fd(), buffer, total_size, size - total_size, len);
			return -1;
		}

		if (len < 0) {
			if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK)) {
				SLEEP_10_MS;
				continue;
			}

			_ERRNO(errno, _E, "Failed to recv(%d, %#p, %x, %d) = %d",
					get_fd(), buffer, total_size, size - total_size, len);
			return -errno;
		}

		total_size += len;
	} while (total_size < size);

	return total_size;
}
