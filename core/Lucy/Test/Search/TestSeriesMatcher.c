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

#define C_LUCY_TESTSERIESMATCHER
#include "Lucy/Util/ToolSet.h"
#include <math.h>

#include "Lucy/Test.h"
#include "Lucy/Test/Search/TestSeriesMatcher.h"
#include "Lucy/Search/BitVecMatcher.h"
#include "Lucy/Search/SeriesMatcher.h"

static SeriesMatcher*
S_make_series_matcher(I32Array *doc_ids, I32Array *offsets, int32_t doc_max) {
    int32_t  num_doc_ids  = I32Arr_Get_Size(doc_ids);
    int32_t  num_matchers = I32Arr_Get_Size(offsets);
    VArray  *matchers     = VA_new(num_matchers);
    int32_t  tick         = 0;
    int32_t  i;

    // Divvy up doc_ids by segment into BitVectors.
    for (i = 0; i < num_matchers; i++) {
        int32_t offset = I32Arr_Get(offsets, i);
        int32_t max    = i == num_matchers - 1
                         ? doc_max + 1
                         : I32Arr_Get(offsets, i + 1);
        BitVector *bit_vec = BitVec_new(max - offset);
        while (tick < num_doc_ids) {
            int32_t doc_id = I32Arr_Get(doc_ids, tick);
            if (doc_id > max) { break; }
            else               { tick++; }
            BitVec_Set(bit_vec, doc_id - offset);
        }
        VA_Push(matchers, (Obj*)BitVecMatcher_new(bit_vec));
        DECREF(bit_vec);
    }

    {
        SeriesMatcher *series_matcher = SeriesMatcher_new(matchers, offsets);
        DECREF(matchers);
        return series_matcher;
    }
}

static I32Array*
S_generate_match_list(int32_t first, int32_t max, int32_t doc_inc) {
    int32_t  count     = (int32_t)ceil(((float)max - first) / doc_inc);
    int32_t *doc_ids   = (int32_t*)MALLOCATE(count * sizeof(int32_t));
    int32_t  doc_id    = first;
    int32_t  i         = 0;

    for (; doc_id < max; doc_id += doc_inc, i++) {
        doc_ids[i] = doc_id;
    }
    if (i != count) { THROW(ERR, "Screwed up somehow: %i32 %i32", i, count); }

    return I32Arr_new_steal(doc_ids, count);
}

static void
S_do_test_matrix(TestBatch *batch, int32_t doc_max, int32_t first_doc_id,
                 int32_t doc_inc, int32_t offset_inc) {
    I32Array *doc_ids
        = S_generate_match_list(first_doc_id, doc_max, doc_inc);
    I32Array *offsets
        = S_generate_match_list(0, doc_max, offset_inc);
    SeriesMatcher *series_matcher
        = S_make_series_matcher(doc_ids, offsets, doc_max);
    uint32_t num_in_agreement = 0;
    int32_t got;

    while (0 != (got = SeriesMatcher_Next(series_matcher))) {
        if (got != I32Arr_Get(doc_ids, num_in_agreement)) { break; }
        num_in_agreement++;
    }
    TEST_INT_EQ(batch, num_in_agreement, I32Arr_Get_Size(doc_ids),
                "doc_max=%d first_doc_id=%d doc_inc=%d offset_inc=%d",
                doc_max, first_doc_id, doc_inc, offset_inc);

    DECREF(doc_ids);
    DECREF(offsets);
    DECREF(series_matcher);
}

static void
test_matrix(TestBatch *batch) {
    int32_t doc_max_nums[]     = { 10, 100, 1000, 0 };
    int32_t first_doc_ids[]    = { 1, 2, 10, 0 };
    int32_t doc_inc_nums[]     = { 20, 13, 9, 4, 2, 0 };
    int32_t offset_inc_nums[]  = { 7, 29, 71, 0 };
    int32_t a, b, c, d;

    for (a = 0; doc_max_nums[a] != 0; a++) {
        for (b = 0; first_doc_ids[b] != 0; b++) {
            for (c = 0; doc_inc_nums[c] != 0; c++) {
                for (d = 0; offset_inc_nums[d] != 0; d++) {
                    int32_t doc_max        = doc_max_nums[a];
                    int32_t first_doc_id   = first_doc_ids[b];
                    int32_t doc_inc        = doc_inc_nums[c];
                    int32_t offset_inc     = offset_inc_nums[d];
                    if (first_doc_id > doc_max) {
                        continue;
                    }
                    else {
                        S_do_test_matrix(batch, doc_max, first_doc_id,
                                         doc_inc, offset_inc);
                    }
                }
            }
        }
    }
}

void
TestSeriesMatcher_run_tests() {
    TestBatch *batch = TestBatch_new(135);
    TestBatch_Plan(batch);
    test_matrix(batch);
    DECREF(batch);
}


