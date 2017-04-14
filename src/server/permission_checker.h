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

#ifndef __PERMISSION_CHECKER_H__
#define __PERMISSION_CHECKER_H__

#include <sensor_types.h>
#include <string>

namespace sensor {

class permission_checker {
public:
	permission_checker();
	~permission_checker();

	bool has_permission(int sock_fd, std::string &perm);

private:
	void init_cynara(void);
	void deinit_cynara(void);
	bool has_permission_cynara(int sock_fd, std::string &perm);
};

}

#endif /* __PERMISSION_CHECKER_H__ */
