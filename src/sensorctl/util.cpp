/*
 * sensorctl
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#include <stdlib.h>
#include "util.h"

bool util::is_number(const char *value)
{
	if (value == NULL || *value == 0)
		return false;

	while (*value) {
		if (*value < '0' || *value > '9')
			return false;
		value++;
	}

	return true;
}

bool util::is_hex(const char *value)
{
	if (value == NULL || *value == 0)
		return false;

	if (value[0] != '0')
		return false;

	if (value[1] != 'x' && value[1] != 'X')
		return false;

	value += 2;

	while (*value) {
		if ((*value < '0' || *value > '9') &&
			(*value < 'a' || *value > 'f') &&
			(*value < 'A' || *value > 'F'))
			return false;
		value++;
	}

	return true;
}
