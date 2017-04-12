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

#include "log.h"
#include "test_bench.h"

/*
 * Implementation of test_option
 */
bool test_option::full_log = false;
std::string test_option::group = "";

void test_option::show_full_log(bool show)
{
	full_log = show;
}

void test_option::set_group(const char *gname)
{
	group = gname;
}

void test_option::set_options(int argc, char *argv[])
{
	/* TODO: use getopt() */
	if (argc > 3)
		set_group(argv[3]);
	if (argc > 4)
		show_full_log(atoi(argv[4]));
}

/*
 * Implementation of test_case
 */
test_case::test_case(const std::string &group, const std::string &name)
: m_group(group)
, m_name(name)
, m_func(NULL)
{
	test_bench::register_testcase(group, this);
}

void test_case::started(void)
{
	_I("[----------]\n");
	_I("[ RUN      ] ");
	_N("%s.%s\n", m_group.c_str(), m_name.c_str());
}

void test_case::stopped(void)
{
	_I("[       OK ] ");
	_N("%s.%s\n", m_group.c_str(), m_name.c_str());
	_I("[----------]\n");
}

void test_case::show(bool result)
{
	if (result)
		_I("[  PASSED  ] ");
	else
		_E("[  FAILED  ] ");
	_N("%s.%s\n", m_group.c_str(), m_name.c_str());
}

void test_case::run_testcase(void)
{
	bool result;

	started();
	result = (this->*m_func)();
	stopped();
	show(result);
}

void test_case::register_func(test_func func)
{
	m_func = func;
}

/*
 * Implementation of test_bench
 */
test_bench& test_bench::instance()
{
	static test_bench bench;
	return bench;
}

void test_bench::register_testcase(const std::string &group, test_case *testcase)
{
	instance().add_testcase(group, testcase);
}

void test_bench::push_failure(const std::string &function, long line, const std::string &msg)
{
	instance().add_failure(function, line, msg);
}

void test_bench::run_all_testcase(void)
{
	instance().run();
}

void test_bench::stop_all_testcase(void)
{
	instance().stop();
}

void test_bench::add_testcase(const std::string &group, test_case *testcase)
{
	if (!testcase)
		return;

	testcases.insert(std::pair<const std::string, test_case *>(group, testcase));
}

void test_bench::add_failure(const std::string &function, long line, const std::string &msg)
{
	test_result fail(function, line, msg);

	results.push_back(fail);
	m_failure_count++;
}

void test_bench::started(void)
{
	_I("[==========] ");
	_N("Running %d testcases\n", count(test_option::group));
}

void test_bench::stopped(void)
{
	_I("[==========] ");
	_N("%d testcases ran\n", count(test_option::group));
}

void test_bench::show_failures(void)
{
	_N("================================\n");

	if (m_failure_count == 0) {
		_N("there was no fail case\n");
		return;
	}

	_N("%d case(s) are failed, listed below:\n", m_failure_count);

	for (unsigned int i = 0; i < results.size(); ++i) {
		_E("[  FAILED  ] ");
		_N("%s\n", results[i].msg.c_str());
	}
}

void test_bench::run(void)
{
	std::size_t found;
	m_failure_count = 0;

	started();

	for (auto it = testcases.begin(); it != testcases.end(); ++it) {
		if (m_stop)
			break;

		found = it->first.find("skip");

		if (test_option::group.empty() && found != std::string::npos)
			continue;

		found = it->first.find(test_option::group);

		if (!test_option::group.empty() && found == std::string::npos)
			continue;

		it->second->run_testcase();
	}

	stopped();
	show_failures();
}

void test_bench::stop(void)
{
	m_stop = true;
}

unsigned int test_bench::count(std::string &group)
{
	if (group.empty())
		return testcases.size() - count_by_key("skip");

	return count_by_key(group.c_str());
}

unsigned int test_bench::count_by_key(const char *key)
{
	int count = 0;

	for (auto it = testcases.begin(); it != testcases.end(); ++it) {
		std::size_t found = it->first.find(key);

		if (found != std::string::npos)
			count++;
	}

	return count;
}
