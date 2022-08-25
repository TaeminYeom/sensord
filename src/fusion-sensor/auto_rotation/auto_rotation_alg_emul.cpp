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

#include "auto_rotation_alg_emul.h"

#include <math.h>
#include <stdlib.h>
#include <sensor_log.h>
#include <sensor_types.h>
#include <libsyscommon/ini-parser.h>

#define ROTATION_RULE_CNT       4
#define AUTO_ROTATION_CONF_PATH "/etc/sensord/auto_rotation.conf"

static struct auto_rotation_config auto_rotation_conf = {
	.fix = false,
	.value = 0,
};

struct rotation_rule {
	int tilt;
	int angle;
};

struct rotation_rule rot_rule[ROTATION_RULE_CNT] = {
	{40, 80},
	{50, 70},
	{60, 65},
	{90, 60},
};

int auto_rotation_load_config(struct parse_result *result, void *user_data)
{
	struct auto_rotation_config *c = (struct auto_rotation_config*) user_data;

	if (!MATCH(result->section, "AutoRotation"))
		return 0;

	if (MATCH(result->name, "Fix"))
		c->fix = (MATCH(result->value, "yes") ? 1 : 0);
	else if (MATCH(result->name, "DefaultValue"))
		SET_CONF(c->value, atoi(result->value));

	return 0;
}

auto_rotation_alg_emul::auto_rotation_alg_emul()
{
	int ret = config_parse(AUTO_ROTATION_CONF_PATH, auto_rotation_load_config, &auto_rotation_conf);
	if (ret < 0)
		_D("Failed to load '%s', so use default config", AUTO_ROTATION_CONF_PATH);
}

auto_rotation_alg_emul::~auto_rotation_alg_emul()
{
}

int auto_rotation_alg_emul::convert_rotation(int prev_rotation,
		float acc_pitch, float acc_theta)
{
	const int ROTATION_0 = 0;
	const int ROTATION_90 = 90;
	const int ROTATION_180 = 180;
	const int ROTATION_360 = 360;
	const int TILT_MIN = 30;
	int tilt;
	int angle;

	int new_rotation = AUTO_ROTATION_DEGREE_UNKNOWN;

	for (int i = 0; i < ROTATION_RULE_CNT; ++i) {
		tilt = rot_rule[i].tilt;

		if ((acc_pitch < TILT_MIN) || (acc_pitch > tilt))
			continue;

		if ((prev_rotation == AUTO_ROTATION_DEGREE_0) || (prev_rotation == AUTO_ROTATION_DEGREE_180))
			angle = rot_rule[i].angle;
		else
			angle = ROTATION_90 - rot_rule[i].angle;

		if ((acc_theta >= ROTATION_360 - angle && acc_theta <= ROTATION_360 - 1) ||
			(acc_theta >= ROTATION_0 && acc_theta <= ROTATION_0 + angle))
			new_rotation = AUTO_ROTATION_DEGREE_0;
		else if (acc_theta >= ROTATION_0 + angle && acc_theta <= ROTATION_180 - angle)
			new_rotation = AUTO_ROTATION_DEGREE_90;
		else if (acc_theta >= ROTATION_180 - angle && acc_theta <= ROTATION_180 + angle)
			new_rotation = AUTO_ROTATION_DEGREE_180;
		else if (acc_theta >= ROTATION_180 + angle && acc_theta <= ROTATION_360 - angle)
			new_rotation = AUTO_ROTATION_DEGREE_270;

		break;
	}

	return new_rotation;
}

bool auto_rotation_alg_emul::get_rotation(float acc[3],
		unsigned long long timestamp, int prev_rotation, int &cur_rotation)
{
	if (auto_rotation_conf.fix) {
		if (auto_rotation_conf.value == 0) {
			cur_rotation = AUTO_ROTATION_DEGREE_0;
		}
		else if (auto_rotation_conf.value == 90) {
			cur_rotation = AUTO_ROTATION_DEGREE_90;
		}
		else if (auto_rotation_conf.value == 180) {
			cur_rotation = AUTO_ROTATION_DEGREE_180;
		}
		else if (auto_rotation_conf.value == 270) {
			cur_rotation = AUTO_ROTATION_DEGREE_270;
		}

		if (prev_rotation == 0)
			return true;
		else
			return false;
	}

	const int ROTATION_90 = 90;
	const int RADIAN = 57.29747;

	double atan_value;
	int acc_theta;
	int acc_pitch;
	double realg;
	float x, y, z;

	x = acc[0];
	y = acc[1];
	z = acc[2];

	atan_value = atan2(x, y);
	acc_theta = (int)(atan_value * (RADIAN) + 360) % 360;
	realg = (double)sqrt((x * x) + (y * y) + (z * z));
	acc_pitch = ROTATION_90 - abs((int) (asin(z / realg) * RADIAN));

	int new_rotation = convert_rotation(prev_rotation, acc_pitch, acc_theta);

	if (new_rotation == AUTO_ROTATION_DEGREE_UNKNOWN) {
		if (prev_rotation == AUTO_ROTATION_DEGREE_UNKNOWN) {
			if (auto_rotation_conf.value == 0) {
				cur_rotation = AUTO_ROTATION_DEGREE_0;
			}
			else if (auto_rotation_conf.value == 90) {
				cur_rotation = AUTO_ROTATION_DEGREE_90;
			}
			else if (auto_rotation_conf.value == 180) {
				cur_rotation = AUTO_ROTATION_DEGREE_180;
			}
			else if (auto_rotation_conf.value == 270) {
				cur_rotation = AUTO_ROTATION_DEGREE_270;
			}
			else {
				cur_rotation = AUTO_ROTATION_DEGREE_0; /* if there is no conf, default degree is 0 */
			}
			return true;
		}

		return false;
	}

	if (new_rotation != prev_rotation) {
		cur_rotation = new_rotation;
		return true;
	}

	return false;
}
