/*
 * sensord
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

#ifndef _ENUM_FACTORY_H_
#define _ENUM_FACTORY_H_

#define ENUM_SENSOR(name) name,
#define ENUM_SENSOR_VALUE(name, assign) name = (assign),

#define ENUM_CASE(name) case (name): return #name;
#define ENUM_CASE_VALUE(name, assign) ENUM_CASE(name)

#define DECLARE_SENSOR_ENUM(ENUM_TYPE, ENUM_DEF) \
	typedef enum ENUM_TYPE { \
		ENUM_DEF(ENUM_SENSOR, ENUM_SENSOR_VALUE) \
	} ENUM_TYPE;

#define DECLARE_SENSOR_ENUM_UTIL_NS(ENUM_TYPE) \
	namespace util_##ENUM_TYPE { \
		const char *get_string(ENUM_TYPE type); \
	};

#define DECLARE_SENSOR_ENUM_UTIL(ENUM_TYPE, ENUM_DEF) \
	const char *util_##ENUM_TYPE::get_string(ENUM_TYPE type) { \
		switch (type) { \
		ENUM_DEF(ENUM_CASE, ENUM_CASE_VALUE) \
		} \
		return "UNKNOWN"; \
	}

#endif /* _ENUM_FACTORY_H_ */
