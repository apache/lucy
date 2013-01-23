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
#include "CFCFileSpec.h"
#include "CFCTest.h"

static void
S_run_tests(CFCTest *test);

const CFCTestBatch CFCTEST_BATCH_FILE_SPEC = {
    "Clownfish::CFC::Model::FileSpec",
    4,
    S_run_tests
};

static void
S_run_tests(CFCTest *test) {
    {
        CFCFileSpec *file_spec
            = CFCFileSpec_new("Clownfish/_include", "Stuff/Thing", 0);
        STR_EQ(test, CFCFileSpec_get_source_dir(file_spec),
               "Clownfish/_include", "get_source_dir");
        STR_EQ(test, CFCFileSpec_get_path_part(file_spec),
               "Stuff/Thing", "get_path_part");
        OK(test, !CFCFileSpec_included(file_spec), "not included");

        CFCBase_decref((CFCBase*)file_spec);
    }

    {
        CFCFileSpec *file_spec
            = CFCFileSpec_new("Clownfish/_include", "Stuff/Thing", 1);
        OK(test, CFCFileSpec_included(file_spec), "included");

        CFCBase_decref((CFCBase*)file_spec);
    }
}

