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

#include "test_bench.h"

#include <getopt.h>
#include <regex>

#include "log.h"

/*
 * Implementation of test_option
 */
bool test_option::verbose = false;
bool test_option::shuffle = false;
bool test_option::show_list = false;
int test_option::repeat = 1;
std::string test_option::filter = "";
std::string test_option::output = "";
int test_option::interval = -1;
int test_option::latency = -1;
int test_option::powersave = -1;

bool test_option::set_options(int argc, char *argv[])
{
	int c;

	while (1) {
		static struct option options[] = {
			{"list", no_argument, 0, 'l'},
			{"filter", required_argument, 0, 'f'},
			{"verbose", no_argument, 0, 'v'},
			{"shuffle", no_argument, 0, 's'},
			{"repeat", required_argument, 0, 'r'},
			{"output", required_argument, 0, 'o'},

			/* For manual test*/
			{"interval", required_argument, 0, 'i'},
			{"batch_latency", required_argument, 0, 'b'},
			{"powersave", required_argument, 0, 'p'},

			{"help", no_argument, 0, 'h'},
			{0, 0, 0, 0}
		};

		int option_index = 0;
		c = getopt_long(argc, argv, "lfvsroibph:", options, &option_index);
		if (c == -1)
			return true;

		switch (c) {
		case 0:
			break;
		case 'l':
			_I("== Testcase List ==\n");
			test_bench::show_testcases();
			test_option::show_list = true;
			break;
		case 'f':
			_I("Filter : %s\n", optarg);
			if (!optarg) break;
			test_option::filter = optarg;
			break;
		case 'v':
			_I("Verbose is on\n");
			test_option::verbose = true;
			break;
		case 's':
			_I("Shuffle is on(Default seed)\n");
			test_option::shuffle = true;
			break;
		case 'r':
			_I("Repeat : %s\n", optarg);
			if (!optarg) break;
			test_option::repeat = atoi(optarg);
			break;
		case 'o':
			/* [TODO] */
			_W("File output is not supported yet, use $sensorctl > out : %s\n", optarg);
			if (!optarg) break;
			test_option::output = optarg;
			break;
		case 'i':
			_I("Interval : %s\n", optarg);
			if (!optarg) break;
			test_option::interval = atoi(optarg);
			break;
		case 'b':
			_I("Batch latency : %s\n", optarg);
			if (!optarg) break;
			test_option::latency = atoi(optarg);
			break;
		case 'p':
			_I("Power save : %s\n", optarg);
			if (!optarg) break;
			test_option::powersave = atoi(optarg);
			break;
		case 'h':
		case '?':
		default:
			return false;
		}
	}

	return true;
}

/*
 * Implementation of test_case
 */
test_case::test_case(const std::string &group, const std::string &name)
: m_group(group)
, m_name(name)
, m_fullname(group + "." + m_name)
, m_func(NULL)
{
	test_bench::register_testcase(group, this);
}

void test_case::started(void)
{
	_I("[ RUN      ] ");
	_N("%s\n", m_fullname.c_str());
}

void test_case::stopped(bool result)
{
	if (result)
		_I("[       OK ] ");
	else
		_E("[  FAILED  ] ");
	_N("%s\n", m_fullname.c_str());
}

void test_case::run_testcase(void)
{
	bool result;

	started();
	result = (this->*m_func)();
	stopped(result);
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

void test_bench::show_testcases(void)
{
	instance().show();
}

void test_bench::register_testcase(const std::string &group, test_case *testcase)
{
	instance().add_testcase(group, testcase);
}

void test_bench::push_failure(const std::string &function, long line, const std::string &msg)
{
	instance().add_failure(function, line, msg);
}

void test_bench::run_all_testcases(void)
{
	if (test_option::show_list)
		return;

	instance().run();
}

void test_bench::stop_all_testcases(void)
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
	results.emplace_back(function, line, msg);
	m_failure_count++;
}

void test_bench::started(void)
{
	_I("[==========] ");
	_N("Running %d testcases\n", count());
}

void test_bench::stopped(void)
{
	_I("[==========] ");
	_N("%d testcases ran\n", count());
}

void test_bench::show(void)
{
	/*
	 * [group1]
	 *     [tc name1]
	 *     [tc name2]
	 * [group2]
	 *     [tc name1]
	 */

	for (auto it = testcases.begin(); it != testcases.end();
			it = testcases.upper_bound(it->first)) {
		auto range = testcases.equal_range(it->first);
		_I("[%s]\n", it->first.c_str());

		for (auto testcase = range.first; testcase != range.second; ++testcase)
			_N("    * %s\n", testcase->second->name().c_str());
	}

	_I("Testcase Count : %u\n", count());
}

void test_bench::show_failures(void)
{
	_N("================================\n");

	if (m_failure_count == 0) {
		_I("[  PASSED  ] ");
		_N("%d tests\n", count() - m_failure_count);
		return;
	}

	_N("%d case(s) are failed, listed below:\n", m_failure_count);

	for (unsigned int i = 0; i < results.size(); ++i) {
		_E("[  FAILED  ] ");
		_N("%s\n", results[i].msg.c_str());
	}
}

bool test_bench::filter(const std::string &name)
{
	static std::regex filter(test_option::filter.c_str(), std::regex::optimize);
	if (!std::regex_match(name, filter)) {
		//_W("Not Matched : %s(%s)\n", name.c_str(), test_option::filter.c_str());
		return false;
	}

	//_I("Matched : %s(%s)\n", name.c_str(), test_option::filter.c_str());
	return true;
}

void test_bench::run(void)
{
	m_failure_count = 0;

	started();

	/* For group */
	for (auto it = testcases.begin(); it != testcases.end();
			it = testcases.upper_bound(it->first)) {
		if (m_stop) break;
		if (!filter(it->second->fullname())) continue;

		auto range = testcases.equal_range(it->first);

		/* Time measurement for test group */
		clock_t start = clock();

		_I("[----------] %d tests from %s\n", testcases.count(it->first), it->first.c_str());
		for (auto testcase = range.first; testcase != range.second; ++testcase) {
			if (m_stop) break;
			testcase->second->run_testcase();
		}

		_I("[----------] %d tests from %s (%.4f sec)\n",
			testcases.count(it->first), it->first.c_str(),
			(double)(clock() - start)/ CLOCKS_PER_SEC);
	}

	stopped();
	show_failures();
}

void test_bench::stop(void)
{
	m_stop = true;
}

unsigned int test_bench::count(void)
{
	int count = 0;

	for (auto it = testcases.begin(); it != testcases.end(); ++it) {
		if (!filter(it->second->fullname()))
			continue;

		count++;
	}

	return count;
}
