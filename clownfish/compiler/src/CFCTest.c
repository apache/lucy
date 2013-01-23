/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "charmony.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define CFC_NEED_BASE_STRUCT_DEF
#define CFC_USE_TEST_MACROS
#include "CFCTest.h"
#include "CFCBase.h"
#include "CFCParser.h"
#include "CFCUtil.h"

typedef struct CFCTestFormatter {
    void (*batch_prologue)(const CFCTestBatch *batch);
    void (*vtest_result)(int pass, int test_num, const char *fmt,
                         va_list args);
    void (*test_comment)(const char *fmt, ...);
    void (*batch_comment)(const char *fmt, ...);
    void (*summary)(const CFCTest *test);
} CFCTestFormatter;

struct CFCTest {
    CFCBase base;
    const CFCTestFormatter *formatter;
    int num_tests;
    int num_tests_failed;
    int num_batches;
    int num_batches_failed;
    int num_tests_in_batch;
    int num_failed_in_batch;
};

static int
S_do_run_batch(CFCTest *self, const CFCTestBatch *batch);

static void
S_vtest_true(CFCTest *self, int cond, const char *fmt, va_list args);

static void
S_format_cfish_batch_prologue(const CFCTestBatch *batch);

static void
S_format_cfish_vtest_result(int pass, int test_num, const char *fmt,
                            va_list args);

static void
S_format_cfish_test_comment(const char *fmt, ...);

static void
S_format_cfish_batch_comment(const char *fmt, ...);

static void
S_format_cfish_summary(const CFCTest *test);

static void
S_format_tap_batch_prologue(const CFCTestBatch *batch);

static void
S_format_tap_vtest_result(int pass, int test_num, const char *fmt,
                          va_list args);

static void
S_format_tap_test_comment(const char *fmt, ...);

static void
S_format_tap_batch_comment(const char *fmt, ...);

static void
S_format_tap_summary(const CFCTest *test);

static const CFCMeta CFCTEST_META = {
    "Clownfish::CFC::Test",
    sizeof(CFCTest),
    (CFCBase_destroy_t)CFCTest_destroy
};

static const CFCTestFormatter S_formatter_cfish = {
    S_format_cfish_batch_prologue,
    S_format_cfish_vtest_result,
    S_format_cfish_test_comment,
    S_format_cfish_batch_comment,
    S_format_cfish_summary
};

static const CFCTestFormatter S_formatter_tap = {
    S_format_tap_batch_prologue,
    S_format_tap_vtest_result,
    S_format_tap_test_comment,
    S_format_tap_batch_comment,
    S_format_tap_summary
};

static const CFCTestBatch *const S_batches[] = {
    &CFCTEST_BATCH_UTIL,
    &CFCTEST_BATCH_DOCU_COMMENT,
    &CFCTEST_BATCH_SYMBOL,
    &CFCTEST_BATCH_VERSION,
    &CFCTEST_BATCH_TYPE,
    &CFCTEST_BATCH_FUNCTION,
    &CFCTEST_BATCH_METHOD,
    &CFCTEST_BATCH_VARIABLE,
    &CFCTEST_BATCH_PARAM_LIST,
    &CFCTEST_BATCH_FILE_SPEC,
    &CFCTEST_BATCH_CLASS,
    &CFCTEST_BATCH_C_BLOCK,
    &CFCTEST_BATCH_PARCEL,
    &CFCTEST_BATCH_FILE,
    &CFCTEST_BATCH_HIERARCHY,
    &CFCTEST_BATCH_PARSER,
    NULL
};

CFCTest*
CFCTest_new(const char *formatter_name) {
    CFCTest *self = (CFCTest*)CFCBase_allocate(&CFCTEST_META);
    return CFCTest_init(self, formatter_name);
}

CFCTest*
CFCTest_init(CFCTest *self, const char *formatter_name) {
    if (strcmp(formatter_name, "clownfish") == 0) {
        self->formatter = &S_formatter_cfish;
    }
    else if (strcmp(formatter_name, "tap") == 0) {
        self->formatter = &S_formatter_tap;
    }
    else {
        CFCUtil_die("Unknown formatter name '%s'", formatter_name);
    }

    self->num_tests           = 0;
    self->num_tests_failed    = 0;
    self->num_batches         = 0;
    self->num_batches_failed  = 0;
    self->num_tests_in_batch  = 0;
    self->num_failed_in_batch = 0;

    return self;
}

void
CFCTest_destroy(CFCTest *self) {
    CFCBase_destroy((CFCBase*)self);
}

int
CFCTest_run_all(CFCTest *self) {
    int failed = 0;

    for (int i = 0; S_batches[i]; ++i) {
        if (!S_do_run_batch(self, S_batches[i])) { failed = 1; }
    }

    return !failed;
}

int
CFCTest_run_batch(CFCTest *self, const char *name) {
    for (int i = 0; S_batches[i]; ++i) {
        const CFCTestBatch *batch = S_batches[i];
        if (strcmp(batch->name, name) == 0) {
            return S_do_run_batch(self, batch);
        }
    }

    CFCUtil_die("Test batch '%s' not found", name);
    return 0;
}

static int
S_do_run_batch(CFCTest *self, const CFCTestBatch *batch) {
    self->formatter->batch_prologue(batch);

    batch->run(self);

    int failed = 0;
    void (*comment)(const char *fmt, ...) = self->formatter->batch_comment;

    if (self->num_failed_in_batch > 0) {
        failed = 1;
        comment("%d/%d tests failed.\n", self->num_failed_in_batch,
                self->num_tests_in_batch);
    }
    if (self->num_tests_in_batch != batch->num_planned) {
        failed = 1;
        comment("Bad plan: You planned %d tests but ran %d.\n",
                batch->num_planned, self->num_tests_in_batch);
    }

    if (failed) {
        self->num_batches_failed += 1;
    }

    self->num_batches += 1;
    self->num_tests_in_batch  = 0;
    self->num_failed_in_batch = 0;

    return !failed;
}

void
CFCTest_test_true(CFCTest *self, int cond, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    S_vtest_true(self, cond, fmt, args);
    va_end(args);
}

void
CFCTest_test_string_equals(CFCTest *self, const char *result,
                           const char *expected, const char *fmt, ...) {
    int cond = (strcmp(result, expected) == 0);

    va_list args;
    va_start(args, fmt);
    S_vtest_true(self, cond, fmt, args);
    va_end(args);

    if (!cond) {
        self->formatter->test_comment("Expected '%s', got '%s'.\n",
                                      expected, result);
    }
}

void
CFCTest_test_int_equals(CFCTest *self, long result, long expected,
                        const char *fmt, ...) {
    int cond = (result == expected);

    va_list args;
    va_start(args, fmt);
    S_vtest_true(self, cond, fmt, args);
    va_end(args);

    if (!cond) {
        self->formatter->test_comment("Expected '%ld', got '%ld'.\n",
                                      expected, result);
    }
}

void
CFCTest_skip(CFCTest *self, int num) {
    self->num_tests          += num;
    self->num_tests_in_batch += num;
}

int
CFCTest_finish(CFCTest *self) {
    self->formatter->summary(self);

    return self->num_batches != 0 && self->num_batches_failed == 0;
}

static void
S_vtest_true(CFCTest *self, int cond, const char *fmt, va_list args) {
    self->num_tests          += 1;
    self->num_tests_in_batch += 1;

    if (!cond) {
        self->num_tests_failed    += 1;
        self->num_failed_in_batch += 1;
    }

    self->formatter->vtest_result(cond, self->num_tests_in_batch, fmt, args);
}

static void
S_format_cfish_batch_prologue(const CFCTestBatch *batch) {
    printf("Testing %s...\n", batch->name);
}

static void
S_format_cfish_vtest_result(int pass, int test_num, const char *fmt,
                            va_list args) {
    if (!pass) {
        printf("  Failed test %d: ", test_num);
        vprintf(fmt, args);
        printf("\n");
    }
}

static void
S_format_cfish_test_comment(const char *fmt, ...) {
    printf("    ");

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

static void
S_format_cfish_batch_comment(const char *fmt, ...) {
    printf("  ");

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

static void
S_format_cfish_summary(const CFCTest *test) {
    if (test->num_batches == 0) {
        printf("No tests planned or run.\n");
    }
    else if (test->num_batches_failed == 0) {
        printf("%d batches passed. %d tests passed.\n",
               test->num_batches, test->num_tests);
        printf("Result: PASS\n");
    }
    else {
        printf("%d/%d batches failed. %d/%d tests failed.\n",
               test->num_batches_failed, test->num_batches,
               test->num_tests_failed, test->num_tests);
        printf("Result: FAIL\n");
    }
}

static void
S_format_tap_batch_prologue(const CFCTestBatch *batch) {
    printf("1..%d\n", batch->num_planned);
}

static void
S_format_tap_vtest_result(int pass, int test_num, const char *fmt,
                          va_list args) {
    const char *result = pass ? "ok" : "not ok";
    printf("%s %d - ", result, test_num);
    vprintf(fmt, args);
    printf("\n");
}

static void
S_format_tap_test_comment(const char *fmt, ...) {
    printf("#   ");

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

static void
S_format_tap_batch_comment(const char *fmt, ...) {
    printf("# ");

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

static void
S_format_tap_summary(const CFCTest *test) {
    (void)test; // unused
}

struct CFCParcel*
CFCTest_parse_parcel(CFCTest *test, CFCParser *parser, const char *src) {
    CFCBase *result = CFCParser_parse(parser, src);
    OK(test, result != NULL, "parse '%s'", src);
    STR_EQ(test, CFCBase_get_cfc_class(result),
           "Clownfish::CFC::Model::Parcel", "result class of '%s'", src);
    return (struct CFCParcel*)result;
}

struct CFCType*
CFCTest_parse_type(CFCTest *test, CFCParser *parser, const char *src) {
    CFCBase *result = CFCParser_parse(parser, src);
    OK(test, result != NULL, "parse '%s'", src);
    STR_EQ(test, CFCBase_get_cfc_class(result),
           "Clownfish::CFC::Model::Type", "result class of '%s'", src);
    return (struct CFCType*)result;
}

struct CFCVariable*
CFCTest_parse_variable(CFCTest *test, CFCParser *parser, const char *src) {
    CFCBase *result = CFCParser_parse(parser, src);
    OK(test, result != NULL, "parse '%s'", src);
    STR_EQ(test, CFCBase_get_cfc_class(result),
           "Clownfish::CFC::Model::Variable", "result class of '%s'", src);
    return (struct CFCVariable*)result;
}

struct CFCParamList*
CFCTest_parse_param_list(CFCTest *test, CFCParser *parser, const char *src) {
    CFCBase *result = CFCParser_parse(parser, src);
    OK(test, result != NULL, "parse '%s'", src);
    STR_EQ(test, CFCBase_get_cfc_class(result),
           "Clownfish::CFC::Model::ParamList", "result class of '%s'", src);
    return (struct CFCParamList*)result;
}

struct CFCFunction*
CFCTest_parse_function(CFCTest *test, CFCParser *parser, const char *src) {
    CFCBase *result = CFCParser_parse(parser, src);
    OK(test, result != NULL, "parse '%s'", src);
    STR_EQ(test, CFCBase_get_cfc_class(result),
           "Clownfish::CFC::Model::Function", "result class of '%s'", src);
    return (struct CFCFunction*)result;
}

struct CFCMethod*
CFCTest_parse_method(CFCTest *test, CFCParser *parser, const char *src) {
    CFCBase *result = CFCParser_parse(parser, src);
    OK(test, result != NULL, "parse '%s'", src);
    STR_EQ(test, CFCBase_get_cfc_class(result),
           "Clownfish::CFC::Model::Method", "result class of '%s'", src);
    return (struct CFCMethod*)result;
}

struct CFCClass*
CFCTest_parse_class(CFCTest *test, CFCParser *parser, const char *src) {
    CFCBase *result = CFCParser_parse(parser, src);
    OK(test, result != NULL, "parse class");
    STR_EQ(test, CFCBase_get_cfc_class(result),
           "Clownfish::CFC::Model::Class", "result class");
    return (struct CFCClass*)result;
}

time_t
CFCTest_get_file_mtime(const char *path) {
    struct stat buf;
    if (stat(path, &buf)) {
        CFCUtil_die("Can't stat '%s': %s", path, strerror(errno));
    }
    return buf.st_mtime;
}

#if defined(CHY_HAS_UTIME_H)

#include <utime.h>

void
CFCTest_set_file_times(const char *path, time_t time) {
    struct utimbuf buf;
    buf.actime  = time;
    buf.modtime = time;
    if (utime(path, &buf)) {
        CFCUtil_die("Can't set file time of '%s': %s", path, strerror(errno));
    }
}

#elif defined(CHY_HAS_WINDOWS_H)

#include <windows.h>

void
CFCTest_set_file_times(const char *path, time_t time) {
    HANDLE handle = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, 0, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        CFCUtil_die("Can't open '%s': %u", path, GetLastError());
    }
    uint64_t ticks = 10000000 * (UINT64_C(11644473600) + time);
    FILETIME file_time;
    file_time.dwLowDateTime  = (DWORD)ticks;
    file_time.dwHighDateTime = (DWORD)(ticks >> 32);
    if (!SetFileTime(handle, &file_time, &file_time, &file_time)) {
        CFCUtil_die("Can't set file time of '%s': %u", path, GetLastError());
    }
    CloseHandle(handle);
}

#else

#error Need either utime.h or windows.h

#endif

