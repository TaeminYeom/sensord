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

#ifndef __SENSOR_LOG_H__
#define __SENSOR_LOG_H__

#include <dlog/dlog.h>
#include <sys/types.h>

#ifdef LOG_TAG
	#undef LOG_TAG
#endif
#define LOG_TAG	"SENSOR"

/* Logging and Error Handling */
#define _I SLOGI
#define _D SLOGD
#define _W SLOGW
#define _E SLOGE
#define _SI SECURE_SLOGI
#define _SD SECURE_SLOGD
#define _SW SECURE_SLOGW
#define _SE SECURE_SLOGE

#define _ERRNO(errno, tag, fmt, arg...) \
	do { \
		char buf[1024]; \
		char *error = strerror_r(errno, buf, 1024); \
		if (!error) { \
			_E("Failed to strerror_r()"); \
			break; \
		} \
		tag(fmt" (%s[%d])", ##arg, error, errno); \
	} while (0)

#define warn_if(expr, fmt, arg...) \
	do { if (expr) { _E(fmt, ##arg); } } while (0)

#define ret_if(expr) \
	do { if (expr) { return; } } while (0)

#define retv_if(expr, val) \
	do { if (expr) { return (val); } } while (0)

#define retm_if(expr, fmt, arg...) \
	do { if (expr) { _E(fmt, ##arg); return; } } while (0)

#define retvm_if(expr, val, fmt, arg...) \
	do { if (expr) { _E(fmt, ##arg); return (val); } } while (0)

#define LOG_DUMP(fp, fmt, arg...) \
	do { if (fp) fprintf(fp, fmt, ##arg); else _E(fmt, ##arg); } while (0)

#define log_oom() ({ \
	_E("Out of memory"); \
	-ENOMEM;})


#endif /* __SENSOR_LOG_H__ */
