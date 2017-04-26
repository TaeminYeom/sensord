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

#include "log.h"
#include "mainloop.h"
#include "test_bench.h"

TESTCASE(sensor_api_provider_uri, provider_check_uri)
{
	int err;
	sensord_provider_h provider;

	const char *uri_p1 = "http://example.org/sensor/mysensor_type/mysensor";
	const char *uri_p2 = "http://developer.samsung.com/sensor/mysensor_type/mysensor";
	const char *uri_n1 = "http://tizen.org/sensor/accelerometer/mysensor";
	const char *uri_n2 = "http://tizen.org/mysensor/accelerometer/mysensor";
	const char *uri_n3 = "http:/example.org/sensor/mysensor_type/mysensor";
	const char *uri_n5 = "http://example.org/sensor/mysensor_type";
	const char *uri_n4 = "http://example.org/sensor/mysensor_type/mysensor/mysensor";

	err = sensord_create_provider(uri_p1, &provider);
	EXPECT_EQ(err, 0);
	err = sensord_create_provider(uri_p2, &provider);
	EXPECT_EQ(err, 0);
	err = sensord_create_provider(uri_n1, &provider);
	EXPECT_EQ(err, -EINVAL);
	err = sensord_create_provider(uri_n2, &provider);
	EXPECT_EQ(err, -EINVAL);
	err = sensord_create_provider(uri_n3, &provider);
	EXPECT_EQ(err, -EINVAL);
	err = sensord_create_provider(uri_n4, &provider);
	EXPECT_EQ(err, -EINVAL);
	err = sensord_create_provider(uri_n5, &provider);
	EXPECT_EQ(err, -EINVAL);

	return true;
}
