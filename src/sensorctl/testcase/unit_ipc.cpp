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

#include "shared/channel.h"
#include "shared/channel_handler.h"
#include "shared/ipc_client.h"
#include "shared/ipc_server.h"

#include "log.h"
#include "test_bench.h"

using namespace ipc;

#define MAX_BUF_SIZE 4096
#define TEST_PATH "/run/.sensord_test.socket"
#define SLEEP_1S sleep(1)

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

class test_echo_server_handler : public channel_handler
{
public:
	void connected(channel *ch) {}
	void disconnected(channel *ch) {}
	void read(channel *ch, message &msg)
	{
		char buf[MAX_BUF_SIZE];

		auto reply = message::create();
		RETM_IF(!reply, "Failed to allocate memory");

		msg.disclose(buf);
		reply->enclose(buf, MAX_BUF_SIZE);

		ch->send(reply);
	}
	void read_complete(channel *ch) {}
	void error_caught(channel *ch, int error) {}
};

/* IPC Echo Server */
static bool run_ipc_server_echo(const char *str, int size, int count)
{
	event_loop eloop;

	ipc_server server(TEST_PATH);
	test_echo_server_handler handler;

	server.set_option("max_connection", 10);
	server.set_option(SO_TYPE, SOCK_STREAM);
	server.bind(&handler, &eloop);

	eloop.run(18000);
	server.close();

	return true;
}

class test_client_handler_30_1M : public channel_handler
{
public:
	void connected(channel *ch) {}
	void disconnected(channel *ch) {}
	void read(channel *ch, message &msg) {}
	void read_complete(channel *ch) {}
	void error_caught(channel *ch, int error) {}
};

/* IPC Client Sleep Test(4096Kb * 1024) */
static bool run_ipc_client_sleep_1s(const char *str, int size, int count)
{
	ipc_client client(TEST_PATH);
	test_client_handler_30_1M client_handler;

	channel *ch = client.connect(&client_handler, NULL);
	ASSERT_NE(ch, 0);

	message msg;
	message reply;
	char buf[MAX_BUF_SIZE] = {'1', '1', '1', };

	msg.enclose(buf, MAX_BUF_SIZE);

	SLEEP_1S;
	ch->send_sync(msg);

	/* Test */
	SLEEP_1S;

	ch->read_sync(reply);
	reply.disclose(buf);

	int ret = strncmp(buf, "111", 3);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(reply.size(), MAX_BUF_SIZE);

	ch->disconnect();
	delete ch;

	return true;
}

/* IPC Client With Small Buffer Test(4096Kb * 1024) */
static bool run_ipc_client_small_buffer(const char *str, int size, int count)
{
	ipc_client client(TEST_PATH);
	test_client_handler_30_1M client_handler;

	channel *ch = client.connect(&client_handler, NULL);
	ASSERT_NE(ch, 0);

	int ret;
	message msg;
	message reply;
	char buf[MAX_BUF_SIZE] = {'1', '1', '1', };

	msg.enclose(buf, MAX_BUF_SIZE);

	SLEEP_1S;

	int buf_size;
	ch->get_option(SO_RCVBUF, buf_size);
	_I("Before: Buffer size : %d\n", buf_size);

	ch->set_option(SO_RCVBUF, buf_size/2048);
	ch->get_option(SO_RCVBUF, buf_size);
	_I("After:  Buffer size : %d\n", buf_size);

	for (int i = 0; i < 1024; ++i) {
		ch->send_sync(msg);
		ch->read_sync(reply);
	}

	reply.disclose(buf);

	ret = strncmp(buf, "111", 3);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(reply.size(), MAX_BUF_SIZE);

	ch->disconnect();
	delete ch;

	return true;
}

/* IPC Client Test(4K * 256) */
static bool run_ipc_client_1M(const char *str, int size, int count)
{
	ipc_client client(TEST_PATH);
	test_client_handler_30_1M client_handler;

	channel *ch = client.connect(&client_handler, NULL);
	ASSERT_NE(ch, 0);

	int ret;
	message msg;
	message reply;
	char buf[MAX_BUF_SIZE] = {'1', '1', '1', };

	msg.enclose(buf, MAX_BUF_SIZE);

	SLEEP_1S;

	for (int i = 0; i < 256; ++i) {
		ch->send_sync(msg);
		ch->read_sync(reply);
	}

	reply.disclose(buf);

	ret = strncmp(buf, "111", 3);
	ASSERT_EQ(ret, 0);
	ASSERT_EQ(reply.size(), MAX_BUF_SIZE);

	ch->disconnect();
	delete ch;

	return true;
}

/* IPC Server Test(Not Echo) */
class test_server_handler : public channel_handler
{
public:
	void connected(channel *ch) {}
	void disconnected(channel *ch) {}
	void read(channel *ch, message &msg)
	{
		char buf[MAX_BUF_SIZE];
		msg.disclose(buf);

		auto reply = message::create();
		if (!reply) return;

		reply->enclose("TEXTTEXTTEXTTEXT", 16);
		ch->send(reply);
	}
	void read_complete(channel *ch) {}
	void error_caught(channel *ch, int error) {}
};

static bool run_ipc_server(const char *str, int size, int count)
{
	event_loop eloop;

	ipc_server server(TEST_PATH);
	test_server_handler handler;

	server.set_option("max_connection", 10);
	server.set_option(SO_TYPE, SOCK_STREAM);
	server.bind(&handler, &eloop);

	/* run main loop for 5 seconds */
	eloop.run(5000);

	server.close();

	return true;
}

/* IPC Client Test(Not Echo) */
class test_client_handler : public channel_handler
{
public:
	void connected(channel *ch) {}
	void disconnected(channel *ch) {}
	void read(channel *ch, message &msg) {}
	void read_complete(channel *ch) {}
	void error_caught(channel *ch, int error) {}
};

static bool run_ipc_client_2_channel_message(const char *str, int size, int count)
{
	ipc_client client(TEST_PATH);
	test_client_handler client_handler;
	channel *ch[2];
	int ret;
	message msg;
	message reply;
	char buf[MAX_BUF_SIZE];

	ch[0] = client.connect(&client_handler, NULL);
	ASSERT_NE(ch[0], 0);

	msg.enclose("TESTTESTTEST", 12);
	ch[0]->send_sync(msg);
	SLEEP_1S;
	ch[0]->read_sync(reply);
	reply.disclose(buf);
	ret = strncmp(buf, "TEXTTEXTTEXTTEXT", 16);
	ASSERT_EQ(ret, 0);

	ch[1] = client.connect(&client_handler, NULL);
	ASSERT_NE(ch[1], 0);

	msg.enclose("TESTTESTTEST", 12);
	ch[1]->send_sync(msg);
	SLEEP_1S;
	ch[1]->read_sync(reply);
	reply.disclose(buf);
	ret = strncmp(buf, "TEXTTEXTTEXTTEXT", 16);
	ASSERT_EQ(ret, 0);

	ch[0]->disconnect();
	ch[1]->disconnect();
	delete ch[0];
	delete ch[1];

	return true;
}

static bool run_ipc_client_2_channel(const char *str, int size, int count)
{
	ipc_client client(TEST_PATH);
	test_client_handler client_handler;
	channel *ch[2];

	ch[0] = client.connect(&client_handler, NULL);
	ASSERT_NE(ch[0], 0);
	ch[1] = client.connect(&client_handler, NULL);
	ASSERT_NE(ch[1], 0);
	ASSERT_NE(ch[1], ch[0]);

	ch[0]->disconnect();
	ch[1]->disconnect();
	delete ch[0];
	delete ch[1];

	return true;
}

static bool run_ipc_client(const char *str, int size, int count)
{
	ipc_client client(TEST_PATH);
	test_client_handler client_handler;

	channel *ch = client.connect(&client_handler, NULL);
	ASSERT_NE(ch, 0);

	int ret;
	message msg;
	message reply;
	char buf[MAX_BUF_SIZE];

	msg.enclose("TESTTESTTEST", 12);
	ch->send_sync(msg);

	SLEEP_1S;

	ch->read_sync(reply);
	reply.disclose(buf);

	ret = strncmp(buf, "TEXTTEXTTEXTTEXT", 16);
	ASSERT_EQ(ret, 0);

	ch->disconnect();
	delete ch;

	return true;
}

/**
 * @brief   Test 3 client + 1 client which sleeps 1 seconds
 */
TESTCASE(sensor_ipc, 3_client_with_1s_sleep_client_p)
{
	pid_t pid = run_process(run_ipc_server_echo, NULL, 0, 0);
	EXPECT_GE(pid, 0);

	SLEEP_1S;

	for (int i = 0; i < 3; ++i) {
		pid = run_process(run_ipc_client_1M, NULL, 0, 0);
		EXPECT_GE(pid, 0);
	}

	bool ret = run_ipc_client_sleep_1s(NULL, 0, 0);
	ASSERT_TRUE(ret);

	SLEEP_1S;

	return true;
}

/**
 * @brief   Test 3 client + 1 client which has small recv buffer(2240)
 */
TESTCASE(sensor_ipc, 3_client_with_small_buffer_client_p)
{
	pid_t pid = run_process(run_ipc_server_echo, NULL, 0, 0);
	EXPECT_GE(pid, 0);

	SLEEP_1S;

	for (int i = 0; i < 3; ++i) {
		pid = run_process(run_ipc_client_1M, NULL, 0, 0);
		EXPECT_GE(pid, 0);
	}

	bool ret = run_ipc_client_small_buffer(NULL, 0, 0);
	ASSERT_TRUE(ret);

	SLEEP_1S;

	return true;
}

/**
 * @brief   Test 30 ipc_client with 1M message
 */
TESTCASE(sensor_ipc, 30_client_with_1M_message_p)
{
	pid_t pid = run_process(run_ipc_server_echo, NULL, 0, 0);
	EXPECT_GE(pid, 0);

	SLEEP_1S;

	for (int i = 0; i < 30; ++i) {
		pid = run_process(run_ipc_client_1M, NULL, 0, 0);
		EXPECT_GE(pid, 0);
	}

	bool ret = run_ipc_client_1M(NULL, 0, 0);
	ASSERT_TRUE(ret);

	SLEEP_1S;

	return true;
}

/**
 * @brief   Test 2 channel of 1 client with message
 */
TESTCASE(sensor_ipc, 1_client_with_2_channel_message_p)
{
	pid_t pid = run_process(run_ipc_server, NULL, 0, 0);
	EXPECT_GE(pid, 0);

	SLEEP_1S;

	bool ret = run_ipc_client_2_channel_message(NULL, 0, 0);
	ASSERT_TRUE(ret);

	SLEEP_1S;

	return true;
}

/**
 * @brief   Test 2 channel of 1 client
 */
TESTCASE(sensor_ipc, 1_client_2_channel_simple_p)
{
	pid_t pid = run_process(run_ipc_server, NULL, 0, 0);
	EXPECT_GE(pid, 0);

	SLEEP_1S;

	bool ret = run_ipc_client_2_channel(NULL, 0, 0);
	ASSERT_TRUE(ret);

	SLEEP_1S;

	return true;
}

/**
 * @brief   Test 100 ipc_client
 */
TESTCASE(sensor_ipc, 100_client_p)
{
	pid_t pid = run_process(run_ipc_server, NULL, 0, 0);
	EXPECT_GE(pid, 0);

	SLEEP_1S;

	for (int i = 0; i < 99; ++i) {
		pid = run_process(run_ipc_client, NULL, 0, 0);
		EXPECT_GE(pid, 0);
	}

	bool ret = run_ipc_client(NULL, 0, 0);
	ASSERT_TRUE(ret);

	SLEEP_1S;

	return true;
}

/**
 * @brief   Test 2 ipc_client
 */
TESTCASE(sensor_ipc, 2_client_p)
{
	pid_t pid = run_process(run_ipc_server, NULL, 0, 0);
	EXPECT_GE(pid, 0);

	SLEEP_1S;

	pid = run_process(run_ipc_client, NULL, 0, 0);
	EXPECT_GE(pid, 0);

	bool ret = run_ipc_client(NULL, 0, 0);
	ASSERT_TRUE(ret);

	SLEEP_1S;

	return true;
}

/**
 * @brief   Test ipc_client/ipc_server class
 * @details 1. connect/disconnect
 *          2. send "TEST" message from client to server
 *          3. check that message in server handler
 */
TESTCASE(sensor_ipc, server_client_basic_p)
{
	pid_t pid = run_process(run_ipc_server, NULL, 0, 0);
	EXPECT_GE(pid, 0);

	SLEEP_1S;

	bool ret = run_ipc_client(NULL, 0, 0);
	ASSERT_TRUE(ret);

	SLEEP_1S;

	return true;
}
