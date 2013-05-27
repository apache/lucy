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

#include <stdlib.h>

#include "Clownfish/TestHarness/TestFormatter.h"
#include "Clownfish/TestHarness/TestSuite.h"
#include "Clownfish/Test.h"
#include "Lucy/Test.h"

int
main() {
    cfish_TestFormatter *formatter;
    cfish_TestSuite     *cfish_suite;
    cfish_TestSuite     *lucy_suite;
    bool success = true;

    testcfish_bootstrap_parcel();
    testlucy_bootstrap_parcel();

    formatter   = (cfish_TestFormatter*)cfish_TestFormatterCF_new();
    cfish_suite = testcfish_Test_create_test_suite();
    lucy_suite  = testlucy_Test_create_test_suite();

    success &= Cfish_TestSuite_Run_All_Batches(cfish_suite, formatter);
    success &= Cfish_TestSuite_Run_All_Batches(lucy_suite, formatter);

    CFISH_DECREF(formatter);
    CFISH_DECREF(cfish_suite);
    CFISH_DECREF(lucy_suite);
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

