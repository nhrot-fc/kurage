#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stddef.h>
#include <stdio.h>

#ifdef TEST_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

typedef struct {
  const char *name;
  int (*fn)(void);
} TestCase;

static inline int RunTestSuite(const char *suite_name, const TestCase *tests,
                               size_t count) {
  if (count == 0) {
    printf("[%s] No tests defined\n", suite_name);
    return 0;
  }

  const char *failed[count];
  size_t failure_count = 0;

  for (size_t i = 0; i < count; ++i) {
    if (tests[i].fn() != 0 && failure_count < count) {
      failed[failure_count++] = tests[i].name;
    }
  }

  if (failure_count == 0) {
    printf("[%s] Tests PASSED (%zu/%zu)\n", suite_name, count, count);
    return 0;
  }

  printf("[%s] Tests FAILED (%zu/%zu)\n", suite_name, failure_count, count);
  printf("Failed cases:\n");
  for (size_t i = 0; i < failure_count; ++i) {
    printf("  - %s\n", failed[i]);
  }

  return 1;
}

#define RUN_TEST_SUITE(name_literal, tests_array)                              \
  RunTestSuite((name_literal), (tests_array),                                  \
               sizeof(tests_array) / sizeof((tests_array)[0]))

#endif
