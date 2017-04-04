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

#ifndef __SEQPACKET_SOCKET_H__
#define __SEQPACKET_SOCKET_H__

#include "socket.h"

namespace ipc {

class seqpacket_socket : public socket {
public:
	seqpacket_socket();
	~seqpacket_socket();

	bool create(const std::string &path);

private:
	ssize_t on_send(const void *buffer, size_t size) const;
	ssize_t on_recv(void *buffer, size_t size) const;

};

}

#endif /* __SEQPACKET_SOCKET_H__ */
