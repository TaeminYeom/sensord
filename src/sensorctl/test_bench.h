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

#pragma once /* __TEST_BENCH_H__ */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#define FAIL(left, comp, right) \
do { \
	_E("[   FAIL   ] "); \
	std::ostringstream os; \
	os << __FUNCTION__ << "(" << __LINE__ << ") : " \
		<< #left << "(" << left << ") " \
		<< #comp << " " << #right << "(" << right << ")"; \
	std::cout << os.str() << std::endl; \
	test_bench::push_failure(__FUNCTION__, __LINE__, os.str()); \
} while (0)

#define PASS(left, comp, right) \
do { \
	if (test_option::full_log) { \
		_I("[   PASS   ] "); \
		std::ostringstream os; \
		os << __FUNCTION__ << "(" << __LINE__ << ") : " \
			<< #left << "(" << left << ") " \
			<< #comp << " " << #right << "(" << right << ")"; \
		std::cout << os.str() << std::endl; \
	} \
} while (0)

#define ASSERT(left, comp, right) \
do { \
	if (!((left) comp (right))) { \
		FAIL(left, comp, right); \
		return false; \
	} \
	PASS(left, comp, right); \
} while (0)


#define EXPECT(left, comp, right) \
do { \
	if (!((left) comp (right))) { \
		FAIL(left, comp, right); \
	} else { \
		PASS(left, comp, right); \
	} \
} while (0)

#define ASSERT_TRUE(condition) ASSERT(condition, ==, true)
#define ASSERT_FALSE(condition) ASSERT(condition, ==, false)
#define ASSERT_EQ(left, right) ASSERT(left, ==, right)
#define ASSERT_NE(left, right) ASSERT(left, !=, right)
#define ASSERT_LT(left, right) ASSERT(left, <, right)
#define ASSERT_LE(left, right) ASSERT(left, <=, right)
#define ASSERT_GT(left, right) ASSERT(left, >, right)
#define ASSERT_GE(left, right) ASSERT(left, >=, right)
#define ASSERT_NEAR(left, right, err) \
do { \
	ASSERT(left, >=, (right - (err))); \
	ASSERT(left, <=, (right + (err))); \
} while (0)

#define EXPECT_TRUE(condition) EXPECT(condition, ==, true)
#define EXPECT_FALSE(condition) EXPECT(condition, ==, false)
#define EXPECT_EQ(left, right) EXPECT(left, ==, right)
#define EXPECT_NE(left, right) EXPECT(left, !=, right)
#define EXPECT_LT(left, right) EXPECT(left, <, right)
#define EXPECT_LE(left, right) EXPECT(left, <=, right)
#define EXPECT_GT(left, right) EXPECT(left, >, right)
#define EXPECT_GE(left, right) EXPECT(left, >=, right)
#define EXPECT_NEAR(left, right, err) \
do { \
	EXPECT(left, >=, (right - (err))); \
	EXPECT(left, <=, (right + (err))); \
} while (0)

#define TESTCASE(group, name) \
class test_case_##group_##name : public test_case { \
public: \
	test_case_##group_##name() \
	: test_case(#group, #name) \
	{ \
		register_func(static_cast<test_case::test_func>(&test_case_##group_##name::test)); \
	} \
	bool test(void); \
} test_case_##group_##name##_instance; \
bool test_case_##group_##name::test(void)

/*
 * Declaration of test_option
 */
class test_option {
public:
	static bool full_log;
	static std::string group;

	static void show_full_log(bool show);
	static void set_group(const char *gname);
	static void set_options(int argc, char *argv[]);
};

/*
 * Decloaration of test_result
 */
class test_result {
public:
	test_result(const std::string &_function, long _line, const std::string &_msg)
	: function(_function)
	, line(_line)
	, msg(_msg) { }

	std::string function;
	long line;
	std::string msg;
};

/*
 * Declaration of test_case
 */
class test_case {
public:
	test_case(const std::string &group, const std::string &name);

	void run_testcase(void);

	const std::string& group() const { return m_group; }
	const std::string& name() const { return m_name; }

protected:
	typedef bool (test_case::*test_func)();

	void started(void);
	void stopped(void);
	void show(bool result);
	void register_func(test_func func);

private:
	const std::string m_group;
	const std::string m_name;
	test_func m_func;
};

/*
 * Declaration of test_bench
 */
class test_bench {
public:
	test_bench()
	: m_failure_count(0)
	, m_stop(false)
	{}

	static void register_testcase(const std::string &group, test_case *testcase);

	static void run_all_testcase(void);
	static void stop_all_testcase(void);

	static void push_failure(const std::string &function, long line, const std::string &msg);

private:
	static test_bench& instance();

	void add_failure(const std::string &function, long line, const std::string &msg);

	void started(void);
	void stopped(void);
	void show_failures(void);

	void add_testcase(const std::string &group, test_case *testcase);
	void run(void);
	void stop(void);

	unsigned int count(std::string &group);
	unsigned int count_by_key(const char *key);

	std::multimap<const std::string, test_case *> testcases;
	std::vector<test_result> results;
	int m_failure_count;
	bool m_stop;
};
