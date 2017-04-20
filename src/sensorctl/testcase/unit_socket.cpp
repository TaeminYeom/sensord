/*
 * sensorctl
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

#include <unistd.h>
#include <string.h>
#include <sensor_internal.h>

#include "shared/socket.h"
#include "shared/stream_socket.h"

#include "log.h"
#include "test_bench.h"

using namespace ipc;

#define MAX_BUF_SIZE 4096
#define TEST_PATH "/run/.sensord_test.socket"

typedef bool (*process_func_t)(const char *msg, int size, int count);

static pid_t run_process(process_func_t func, const char *msg, int size, int count)
{
	pid_t pid = fork();
	if (pid < 0)
		return -1;

	if (pid == 0) {
		if (!func(msg, size, count))
			_E("Failed to run process\n");
		exit(0);
	}

	return pid;
}

/* Socket */
static bool run_socket_echo_server(const char *msg, int size, int count)
{
	char buf[MAX_BUF_SIZE] = {0, };
	bool ret = false;
	int send_size = 0;
	int recv_size = 0;
	int recv_count = 0;
	int send_count = 0;
	int total_recv_size = 0;
	int total_send_size = 0;
	stream_socket accept_sock;
	stream_socket client_sock;

	accept_sock.create(TEST_PATH);
	accept_sock.set_blocking_mode(true);
	accept_sock.bind();
	accept_sock.listen(10);
	accept_sock.accept(client_sock);

	/* receive message */
	while (recv_count++ < count) {
		recv_size = client_sock.recv(&buf, size);
		total_recv_size += recv_size;
	}
	ASSERT_EQ(total_recv_size, (size * count));

	usleep(10000);
	/* echo message */
	while (send_count++ < count) {
		send_size = client_sock.send(buf, size);
		total_send_size += send_size;
	}
	ASSERT_EQ(total_send_size, (size * count));

	ret = strncmp(buf, msg, size);
	ASSERT_EQ(ret, 0);

	accept_sock.close();
	client_sock.close();

	return true;
}

static bool run_socket_client(const char *msg, int size, int count)
{
	char buf[MAX_BUF_SIZE] = {0, };
	bool ret = false;
	int send_size = 0;
	int recv_size = 0;
	int send_count = 0;
	int recv_count = 0;
	int total_recv_size = 0;
	int total_send_size = 0;
	stream_socket sock;

	usleep(100000);

	sock.create(TEST_PATH);
	sock.set_blocking_mode(true);
	sock.connect();

	while (send_count++ < count) {
		send_size = sock.send(msg, size);
		total_send_size += send_size;
	}

	ASSERT_EQ(total_send_size, (size * count));

	while (recv_count++ < count) {
		recv_size = sock.recv(&buf, size);
		total_recv_size += recv_size;
	}

	ASSERT_EQ(total_recv_size, (size * count));

	ret = strncmp(buf, msg, size);
	ASSERT_EQ(ret, 0);

	sock.close();

	return true;
}

/**
 * @brief   Test socket class with simple message
 * @details 1. connect socket and listen event by socket
 *          2. send/recv "TEST" text by using echo server
 *          3. check "TEST" message
 * @remarks we can test only regular socket, not systemd-based socket.
 */
TESTCASE(sensor_ipc_socket, socket_p_0)
{
	const char *msg = "TEST";
	int size = 4;
	int count = 1;

	pid_t pid = run_process(run_socket_echo_server, msg, size, count);
	EXPECT_GE(pid, 0);

	bool ret = run_socket_client(msg, size, count);
	ASSERT_TRUE(ret);

	return true;
}

/**
 * @brief   Test socket class with 40K message
 * @details 1. connect socket and listen event by socket
 *          2. send/recv 40960 bytes(4096 bytes * 10) by using echo server
 *          3. check total size
 * @remarks we can test only regular socket, not systemd-based socket.
 */
TESTCASE(sensor_ipc_socket, socket_p_10)
{
	const char msg[MAX_BUF_SIZE] = {1, };
	int size = MAX_BUF_SIZE;
	int count = 10;

	pid_t pid = run_process(run_socket_echo_server, msg, size, count);
	EXPECT_GE(pid, 0);

	bool ret = run_socket_client(msg, size, count);
	ASSERT_TRUE(ret);

	return true;
}

/**
 * @brief   Test socket class with 4M message
 * @details 1. connect socket and listen event by socket
 *          2. send/recv 4096000 bytes(4096 bytes * 1000) by using echo server
 *          3. check total size
 * @remarks we can test only regular socket, not systemd-based socket.
 */
TESTCASE(sensor_ipc_socket, socket_p_1000)
{
	const char msg[MAX_BUF_SIZE] = {1, };
	int size = MAX_BUF_SIZE;
	int count = 1000;

	pid_t pid = run_process(run_socket_echo_server, msg, size, count);
	EXPECT_GE(pid, 0);

	bool ret = run_socket_client(msg, size, count);
	ASSERT_TRUE(ret);

	return true;
}