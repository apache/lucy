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

#include "Clownfish/Test/TestFormatter.h"
#include "Lucy/Test.h"

int
main() {
    cfish_TestFormatterCF *formatter;
    bool success;

    lucy_bootstrap_parcel();

    formatter = cfish_TestFormatterCF_new();
    success = lucy_Test_run_all_batches((cfish_TestFormatter*)formatter);
    CFISH_DECREF(formatter);

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

