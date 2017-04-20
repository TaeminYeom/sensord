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

#include "permission_checker.h"

#include <cynara-client.h>
#include <cynara-creds-socket.h>
#include <cynara-session.h>
#include <sensor_log.h>
#include <unordered_map>

#define CACHE_SIZE 16

using namespace sensor;

static cynara *cynara_env = NULL;
static std::unordered_map<int, std::string> permissions;

permission_checker::permission_checker()
{
	init_cynara();
}

permission_checker::~permission_checker()
{
	deinit_cynara();
}

void permission_checker::init_cynara(void)
{
	int err;
	cynara_configuration *conf = NULL;

	err = cynara_configuration_create(&conf);
	retm_if(err != CYNARA_API_SUCCESS, "Failed to create cynara configuration");

	err = cynara_configuration_set_cache_size(conf, CACHE_SIZE);
	if (err != CYNARA_API_SUCCESS) {
		_E("Failed to set cynara cache");
		cynara_configuration_destroy(conf);
		return;
	}

	err = cynara_initialize(&cynara_env, conf);
	cynara_configuration_destroy(conf);

	if (err != CYNARA_API_SUCCESS) {
		_E("Failed to initialize cynara");
		cynara_env = NULL;
		return;
	}

	_I("Initialized");
}

void permission_checker::deinit_cynara(void)
{
	if (cynara_env) {
		cynara_finish(cynara_env);
		cynara_env = NULL;
	}

	_I("Deinitialized");
}

bool permission_checker::has_permission_cynara(int sock_fd, std::string &perm)
{
	retvm_if(cynara_env == NULL, false, "Cynara not initialized");

	int ret;
	int pid = -1;
	char *client = NULL;
	char *session = NULL;
	char *user = NULL;

	retvm_if(cynara_creds_socket_get_pid(sock_fd, &pid) != CYNARA_API_SUCCESS,
			false, "Failed to get pid");

	if (cynara_creds_socket_get_client(sock_fd,
				CLIENT_METHOD_DEFAULT, &client) != CYNARA_API_SUCCESS ||
			cynara_creds_socket_get_user(sock_fd,
				USER_METHOD_DEFAULT, &user) != CYNARA_API_SUCCESS ||
			(session = cynara_session_from_pid(pid)) == NULL) {
		_E("Failed to get client information");
		free(client);
		free(user);
		free(session);
		return false;
	}

	ret = cynara_check(cynara_env, client, session, user, perm.c_str());

	free(client);
	free(session);
	free(user);

	return (ret == CYNARA_API_ACCESS_ALLOWED);
}

bool permission_checker::has_permission(int sock_fd, std::string &perm)
{
	retv_if(perm.empty(), true);

	return has_permission_cynara(sock_fd, perm);
}
