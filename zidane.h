/*
 * Copyright (c) 2022 Leonardo Duarte
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _ZIDANE_H
#define _ZIDANE_H

#include <stdbool.h>
#include <stdio.h>

#ifndef ZIDANE_SINGLE_THREADED
#include <threads.h>
#endif

#define ZIDANE_VERIFY(p)                                                                \
	do {                                                                            \
		if (!(p)) {                                                             \
			__zidane_current_test_failed = true;                            \
			__zidane_failure_info_array[__zidane_failure_info_array_size] = \
				(struct failure_info) {                                 \
					.file = __FILE__,                               \
					.line = __LINE__,                               \
					.expression = #p,                               \
				};                                                      \
			__zidane_failure_info_array_size += 1;                          \
		}                                                                       \
	} while (0)

#define __ZIDANE_TEST_FUNC_NAME(name) __zidane_test_##name

#define ZIDANE_TEST(name)                                                  \
	static void __ZIDANE_TEST_FUNC_NAME(name)();                       \
	static struct __zidane_test_fn_container *__zidane_test_ptr_##name \
		__attribute((used, section("test_suite_array"))) =         \
			&(struct __zidane_test_fn_container) {             \
				.s = #name,                                \
				.f = __ZIDANE_TEST_FUNC_NAME(name),        \
			};                                                 \
	static void __ZIDANE_TEST_FUNC_NAME(name)()

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*__zidane_test_fn_type)(void);

struct __zidane_test_fn_container {
#ifndef ZIDANE_SINGLE_THREADED
	thrd_t tid;
#endif
	const char *s;
	__zidane_test_fn_type f;
};

struct failure_info {
	const char *file;
	unsigned int line;
	const char *expression;
};

extern _Thread_local bool __zidane_current_test_failed;
extern _Thread_local struct failure_info __zidane_failure_info_array[256];
extern _Thread_local unsigned int __zidane_failure_info_array_size;

extern void zidane_init(void);
extern void zidane_deinit(void);

int main(void);

#ifdef __cplusplus
}
#endif

#endif // _ZIDANE_H

#ifdef ZIDANE_IMPLEMENTATION
#undef ZIDANE_IMPLEMENTATION

#include <sys/time.h>

_Thread_local bool __zidane_current_test_failed = false;
_Thread_local struct failure_info __zidane_failure_info_array[256];
_Thread_local unsigned int __zidane_failure_info_array_size = 0;

extern struct __zidane_test_fn_container *__start_test_suite_array;
extern struct __zidane_test_fn_container *__stop_test_suite_array;

#ifndef ZIDANE_SINGLE_THREADED
static int __zidane_run_test(void *arg)
{
	struct __zidane_test_fn_container **ptr = arg;

	(*ptr)->f();

	flockfile(stderr);
	flockfile(stdout);
	for (unsigned int i = 0; i < __zidane_failure_info_array_size; ++i) {
		fprintf(stderr, "%s:%d: verify failed: %s.\n", __zidane_failure_info_array[i].file,
			__zidane_failure_info_array[i].line,
			__zidane_failure_info_array[i].expression);
	}

	if (__zidane_current_test_failed) {
		printf("Test case \033[31m`%s`\033[0m failed.\n", (*ptr)->s);
	}
	funlockfile(stdout);
	funlockfile(stderr);

	return __zidane_current_test_failed;
}

static void __zidane_run_all_tests(unsigned int *tests_failed_count,
				   unsigned int *tests_passed_count)
{
	for (struct __zidane_test_fn_container **ptr = &__start_test_suite_array;
	     ptr < &__stop_test_suite_array; ++ptr) {
		thrd_t tid;
		thrd_create(&tid, __zidane_run_test, ptr);

		(*ptr)->tid = tid;
	}

	for (struct __zidane_test_fn_container **ptr = &__start_test_suite_array;
	     ptr < &__stop_test_suite_array; ++ptr) {
		int ret;

		thrd_join((*ptr)->tid, &ret);

		if (ret != 0) {
			*tests_failed_count += 1;
		} else {
			*tests_passed_count += 1;
		}
	}
}
#else
static void __zidane_run_all_tests(unsigned int *tests_failed_count,
				   unsigned int *tests_passed_count)
{
	for (struct __zidane_test_fn_container **ptr = &__start_test_suite_array;
	     ptr < &__stop_test_suite_array; ++ptr) {
		(*ptr)->f();

		if (__zidane_current_test_failed) {
			for (unsigned int i = 0; i < __zidane_failure_info_array_size; ++i) {
				fprintf(stderr, "%s:%d: verify failed: %s.\n",
					__zidane_failure_info_array[i].file,
					__zidane_failure_info_array[i].line,
					__zidane_failure_info_array[i].expression);
			}
			printf("Test case \033[31m`%s`\033[0m failed.\n", (*ptr)->s);

			__zidane_current_test_failed = false;
			__zidane_failure_info_array_size = 0;
			*tests_failed_count += 1;
		} else {
			*tests_passed_count += 1;
		}
	}
}
#endif

#ifndef ZIDANE_INIT
void zidane_init(void) {}
#endif

#ifndef ZIDANE_DEINIT
void zidane_deinit(void) {}
#endif

int main(void)
{
	zidane_init();

	unsigned int tests_failed_count = 0;
	unsigned int tests_passed_count = 0;

	bool should_time = true;

	struct timeval start_tv;
	if (should_time) {
		if (gettimeofday(&start_tv, NULL) < 0) {
			should_time = false;
		}
	}

	__zidane_run_all_tests(&tests_failed_count, &tests_passed_count);

	struct timeval end_tv;
	if (should_time) {
		if (gettimeofday(&end_tv, NULL) < 0) {
			should_time = false;
		}
	}

	printf("Tests:\t");

	printf("\033[31m%u failed, \033[0m", tests_failed_count);
	printf("\033[32m%u passed, \033[0m", tests_passed_count);
	printf("%u total\n", tests_failed_count + tests_passed_count);

	if (should_time) {
		double start_time = (long) start_tv.tv_sec * 1000.0 + start_tv.tv_usec / 1000.0;
		double end_time = (long) end_tv.tv_sec * 1000.0 + end_tv.tv_usec / 1000.0;

		// In milliseconds
		double elapsed_time = end_time - start_time;

		if (elapsed_time > 1000.0) {
			printf("Time elapsed: %0.3fs\n", elapsed_time / 1000.0);
		} else {
			printf("Time elapsed: %0.3fms\n", elapsed_time);
		}
	}

	zidane_deinit();

	return 0;
}

#endif // ZIDANE_IMPLEMENTATION
