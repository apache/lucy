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

#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Index/TestIndexManager.h"
#include "Lucy/Index/IndexManager.h"

static void
test_Choose_Sparse(TestBatch *batch) {
    IndexManager *manager = IxManager_new(NULL, NULL);

    for (uint32_t num_segs = 2; num_segs < 20; num_segs++) {
        I32Array *doc_counts = I32Arr_new_blank(num_segs);
        for (uint32_t j = 0; j < num_segs; j++) {
            I32Arr_Set(doc_counts, j, 1000);
        }
        uint32_t threshold = IxManager_Choose_Sparse(manager, doc_counts);
        TEST_TRUE(batch, threshold != 1,
                  "Either don't merge, or merge two segments: %u segs, thresh %u",
                  (unsigned)num_segs, (unsigned)threshold);

        if (num_segs != 12 && num_segs != 13) {  // when 2 is correct
            I32Arr_Set(doc_counts, 0, 1);
            threshold = IxManager_Choose_Sparse(manager, doc_counts);
            TEST_TRUE(batch, threshold != 2,
                      "Don't include big next seg: %u segs, thresh %u",
                      (unsigned)num_segs, (unsigned)threshold);
        }

        DECREF(doc_counts);
    }

    DECREF(manager);
}

void
TestIxManager_run_tests() {
    TestBatch *batch = TestBatch_new(34);
    TestBatch_Plan(batch);
    test_Choose_Sparse(batch);
    DECREF(batch);
}

