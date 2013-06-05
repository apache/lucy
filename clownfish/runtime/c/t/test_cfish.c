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

int
main() {
    cfish_TestFormatter *formatter;
    cfish_TestSuite     *suite;
    bool success;

    testcfish_bootstrap_parcel();

    formatter = (cfish_TestFormatter*)cfish_TestFormatterCF_new();
    suite     = testcfish_Test_create_test_suite();
    success   = Cfish_TestSuite_Run_All_Batches(suite, formatter);

    CFISH_DECREF(formatter);
    CFISH_DECREF(suite);
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

