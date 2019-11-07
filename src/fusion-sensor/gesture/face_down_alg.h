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

#ifndef __FACE_DOWN_ALG_H__
#define __FACE_DOWN_ALG_H__

#include <sensor_types.h>

class face_down_alg {
public:
	virtual ~face_down_alg() {};
	virtual void update(sensor_data_t *data) = 0;
	virtual bool get_face_down(void) = 0;
};

#endif /* __FACE_DOWN_ALG_H __ */
