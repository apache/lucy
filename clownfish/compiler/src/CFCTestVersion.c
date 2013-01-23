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

#define CFC_USE_TEST_MACROS
#include "CFCBase.h"
#include "CFCVersion.h"
#include "CFCTest.h"

static void
S_run_tests(CFCTest *test);

const CFCTestBatch CFCTEST_BATCH_VERSION = {
    "Clownfish::CFC::Model::Version",
    11,
    S_run_tests
};

static void
S_run_tests(CFCTest *test) {
    CFCVersion *v3_2     = CFCVersion_new("v3.2");
    CFCVersion *v3_2_0   = CFCVersion_new("v3.2.0");
    CFCVersion *v3_2_1   = CFCVersion_new("v3.2.1");
    CFCVersion *v3_2_1_0 = CFCVersion_new("v3.2.1.0");
    CFCVersion *v3_3     = CFCVersion_new("v3.3");
    CFCVersion *v90210   = CFCVersion_new("v90210");

    INT_EQ(test, CFCVersion_get_major(v3_2_1), 3, "get_major");
    INT_EQ(test, CFCVersion_get_major(v90210), 90210, "parse big number");
    STR_EQ(test, CFCVersion_get_vstring(v3_2_1), "v3.2.1", "get_vstring");

    int result;
    result = CFCVersion_compare_to(v3_2_1, v3_2_1_0);
    INT_EQ(test, result, 0, "ignore zeroes in compare_to");
    result = CFCVersion_compare_to(v3_2_1_0, v3_2_1);
    INT_EQ(test, result, 0, "ignore zeroes in compare_to");
    result = CFCVersion_compare_to(v3_2_1, v3_3);
    INT_EQ(test, result, -1, "compare_to A < B_fewer_digits");
    result = CFCVersion_compare_to(v3_3, v3_2_1);
    INT_EQ(test, result, 1, "compare_to A_fewer_digits > B");
    result = CFCVersion_compare_to(v3_2_1, v3_2);
    INT_EQ(test, result, 1, "compare_to A < B_fewer_digits");
    result = CFCVersion_compare_to(v3_2, v3_2_1);
    INT_EQ(test, result, -1, "compare_to A_fewer_digits > B");
    result = CFCVersion_compare_to(v3_2_1, v3_2_0);
    INT_EQ(test, result, 1, "compare_to A > B");
    result = CFCVersion_compare_to(v3_2_0, v3_2_1);
    INT_EQ(test, result, -1, "compare_to A < B");

    CFCBase_decref((CFCBase*)v3_2);
    CFCBase_decref((CFCBase*)v3_2_0);
    CFCBase_decref((CFCBase*)v3_2_1);
    CFCBase_decref((CFCBase*)v3_2_1_0);
    CFCBase_decref((CFCBase*)v3_3);
    CFCBase_decref((CFCBase*)v90210);
}

