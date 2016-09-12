/*
 * sensord
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define _cleanup_(x) __attribute__((cleanup(x)))

static inline void __freep(void *p)
{
	free(*(void**) p);
}

static inline void __closep(int *fd)
{
	if (*fd >= 0)
		close(*fd);
}

static inline void __fclosep(FILE **f)
{
	if (*f)
		fclose(*f);
}

#define _cleanup_free_ _cleanup_(__freep)
#define _cleanup_close_ _cleanup_(__closep)
#define _cleanup_fclose_ _cleanup_(__fclosep)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
