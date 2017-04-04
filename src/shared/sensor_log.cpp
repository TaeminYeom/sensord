/*
 * sensord
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
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

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sensor_log.h>
#include <dlog.h>
#include <stddef.h>
#include <sensor_types.h>

#define PATH_MAX 256

#if defined(_DEBUG)
bool get_proc_name(pid_t pid, char *process_name)
{
	FILE *fp;
	char buf[NAME_MAX];
	char filename[PATH_MAX];

	sprintf(filename, "/proc/%d/stat", pid);
	fp = fopen(filename, "r");

	if (fp == NULL)
		return false;

	if (fscanf(fp, "%*s (%[^)]", buf) < 1) {
		fclose(fp);
		return false;
	}

	strncpy(process_name, buf, NAME_MAX-1);
	process_name[NAME_MAX-1] = '\0';
	fclose(fp);

	return true;
}
#else
bool get_proc_name(pid_t pid, char *process_name)
{
	char buf[NAME_MAX];

	if (snprintf(buf, sizeof(buf), "%d process", pid) < 1) {
		return false;
	}

	strncpy(process_name, buf, NAME_MAX-1);
	process_name[NAME_MAX-1] = '\0';

	return true;
}
#endif

const char* get_client_name(void)
{
	const int pid_string_size = 10;
	static pid_t pid = -1;
	static char client_name[NAME_MAX + pid_string_size];

	char proc_name[NAME_MAX];

	if (pid == -1)
	{
		pid = getpid();
		get_proc_name(pid, proc_name);
		snprintf(client_name, sizeof(client_name), "%s(%d)", proc_name, pid);
	}

	return client_name;
}
