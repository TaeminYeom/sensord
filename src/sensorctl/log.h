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

#pragma once /* __LOG_H__ */

#include <stdio.h>
#include <glib.h>

#define _NRM  "\x1B[0m"
#define _RED  "\x1B[31m"
#define _GRN  "\x1B[32m"
#define _YEL  "\x1B[33m"
#define _BLU  "\x1B[34m"
#define _MAG  "\x1B[35m"
#define _CYN  "\x1B[36m"
#define _WHT  "\x1B[37m"
#define _RST "\033[0m"

#define _N(fmt, args...) \
do { \
	g_print(fmt, ##args); \
} while (0)

#define _E(fmt, args...) \
do { \
	g_print("\x1B[31m" fmt "\033[0m", ##args); \
} while (0)

#define _I(fmt, args...) \
do { \
	g_print("\x1B[32m" fmt "\033[0m", ##args); \
} while (0)

#define WARN_IF(expr, fmt, arg...) \
do { \
	if(expr) { \
		_E(fmt, ##arg); \
	} \
} while (0)

#define RET_IF(expr) \
do { \
	if(expr) { \
		return; \
	} \
} while (0)

#define RETV_IF(expr, val) \
do { \
	if(expr) { \
		return (val); \
	} \
} while (0)

#define RETM_IF(expr, fmt, arg...) \
do { \
	if(expr) { \
		_E(fmt, ##arg); \
		return; \
	} \
} while (0)

#define RETVM_IF(expr, val, fmt, arg...) \
do { \
	if(expr) { \
		_E(fmt, ##arg); \
		return (val); \
	} \
} while (0)
