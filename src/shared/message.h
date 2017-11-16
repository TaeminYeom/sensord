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

#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <stdlib.h> /* size_t */
#include <atomic>

#define MAX_MSG_CAPACITY 10240
#define MAX_HEADER_RESERVED 3

namespace ipc {

typedef struct message_header {
	uint64_t id;
	uint32_t type;
	size_t length;
	int32_t err;
	void *ancillary[MAX_HEADER_RESERVED];
} message_header;

class message {
public:
	message(size_t capacity = MAX_MSG_CAPACITY);
	message(const void *msg, size_t size);
	message(const message &msg);
	message(int err);
	~message();

	void enclose(const void *msg, const size_t size);
	void enclose(int error);
	void disclose(void *msg);

	uint32_t type(void);
	void set_type(uint32_t type);

	size_t size(void);

	void ref(void);
	void unref(void);
	int  ref_count(void);

	message_header *header(void);
	char *body(void);

private:
	message_header m_header;
	size_t m_size;
	size_t m_capacity;

	char *m_msg;
	std::atomic<int> ref_cnt;
};

}

#endif /* __MESSAGE_H__ */
