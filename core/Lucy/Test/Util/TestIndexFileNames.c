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

#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Util/TestIndexFileNames.h"
#include "Lucy/Util/IndexFileNames.h"

TestIndexFileNames*
TestIxFileNames_new() {
    return (TestIndexFileNames*)Class_Make_Obj(TESTINDEXFILENAMES);
}

static void
S_test_local_part(TestBatchRunner *runner, const char *source,
                  const char *wanted, const char *test_name) {
    StackString *source_str = SSTR_WRAP_UTF8(source, strlen(source));
    String *got = IxFileNames_local_part((String*)source_str);
    TEST_TRUE(runner, Str_Equals_Utf8(got, wanted, strlen(wanted)), test_name);
    DECREF(got);
}

static void
test_local_part(TestBatchRunner *runner) {
    S_test_local_part(runner, "", "", "simple name");
    S_test_local_part(runner, "foo.txt", "foo.txt", "name with extension");
    S_test_local_part(runner, "/foo", "foo", "strip leading slash");
    S_test_local_part(runner, "/foo/", "foo", "strip trailing slash");
    S_test_local_part(runner, "foo/bar\\ ", "bar\\ ",
                      "Include garbage like backslashes and spaces");
    S_test_local_part(runner, "foo/bar/baz.txt", "baz.txt",
                      "find last component");
}

static void
S_test_extract_gen(TestBatchRunner *runner, const char *name, uint64_t gen,
                   const char *test_name) {
    StackString *source = SSTR_WRAP_UTF8(name, strlen(name));
    TEST_TRUE(runner, IxFileNames_extract_gen((String*)source) == gen,
              test_name);
}

static void
test_extract_gen(TestBatchRunner *runner) {
    S_test_extract_gen(runner, "seg_9", 9, "extract_gen");
    S_test_extract_gen(runner, "seg_9/", 9, "deal with trailing slash");
    S_test_extract_gen(runner, "seg_9_8", 9, "Only go past first underscore");
    S_test_extract_gen(runner, "snapshot_5.json", 5, "Deal with file suffix");
}

void
TestIxFileNames_Run_IMP(TestIndexFileNames *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 10);
    test_local_part(runner);
    test_extract_gen(runner);
}


