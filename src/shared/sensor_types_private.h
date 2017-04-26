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

#ifndef __SENSOR_TYPES_PRIVATE__
#define __SENSOR_TYPES_PRIVATE__

#define URI_REGEX(CATEGORY) R"~(^http:\/\/[\w-]+(\.[\w-]+)*\/)~" CATEGORY R"~(\/[\w-]+(\.[\w-]+)*(\/[\w-]+(\.[\w-]+)*)$)~"
#define SENSOR_URI_REGEX URI_REGEX("sensor")

#define PREDEFINED_TYPE_URI "http://tizen.org/sensor/"

#endif /* __SENSOR_TYPES_PRIVATE__ */
