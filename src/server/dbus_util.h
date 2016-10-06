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

#ifndef _DBUS_UTIL_H_
#define _DBUS_UTIL_H_

#define SENSORD_BUS_NAME              "org.tizen.system.sensord"
#define SENSORD_OBJECT_PATH           "/Org/Tizen/System/SensorD"
#define SENSORD_INTERFACE_NAME        SENSORD_BUS_NAME

enum dbus_ret {
	DBUS_INIT = -1,
	DBUS_FAILED = 0,
	DBUS_SUCCESS = 1
};

int get_lcdon_count(void);
void increase_lcdon_count(void);
void reset_lcdon_count(void);

int get_total_count(void);
void increase_total_count(void);
void reset_total_count(void);
void set_wrist_up_algo(int mode);

void init_dbus(void);
void fini_dbus(void);

#endif /* SENSORD_GDBUS_H_ */
