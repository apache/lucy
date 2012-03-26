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

#define C_LUCY_MATCHER
#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "Lucy/Search/Matcher.h"
#include "Lucy/Object/Err.h"
#include "Lucy/Object/VTable.h"
#include "Lucy/Search/Collector.h"

Matcher*
Matcher_init(Matcher *self) {
    ABSTRACT_CLASS_CHECK(self, MATCHER);
    return self;
}

int32_t
Matcher_advance(Matcher *self, int32_t target) {
    while (1) {
        int32_t doc_id = Matcher_Next(self);
        if (doc_id == 0 || doc_id >= target) {
            return doc_id;
        }
    }
}

void
Matcher_collect(Matcher *self, Collector *collector, Matcher *deletions) {
    int32_t doc_id        = 0;
    int32_t next_deletion = deletions ? 0 : I32_MAX;

    Coll_Set_Matcher(collector, self);

    // Execute scoring loop.
    while (1) {
        if (doc_id > next_deletion) {
            next_deletion = Matcher_Advance(deletions, doc_id);
            if (next_deletion == 0) { next_deletion = I32_MAX; }
            continue;
        }
        else if (doc_id == next_deletion) {
            // Skip past deletions.
            while (doc_id == next_deletion) {
                // Artifically advance matcher.
                while (doc_id == next_deletion) {
                    doc_id++;
                    next_deletion = Matcher_Advance(deletions, doc_id);
                    if (next_deletion == 0) { next_deletion = I32_MAX; }
                }
                // Verify that the artificial advance actually worked.
                doc_id = Matcher_Advance(self, doc_id);
                if (doc_id > next_deletion) {
                    next_deletion = Matcher_Advance(deletions, doc_id);
                }
            }
        }
        else {
            doc_id = Matcher_Advance(self, doc_id + 1);
            if (doc_id >= next_deletion) {
                next_deletion = Matcher_Advance(deletions, doc_id);
                if (doc_id == next_deletion) { continue; }
            }
        }

        if (doc_id) {
            Coll_Collect(collector, doc_id);
        }
        else {
            break;
        }
    }

    Coll_Set_Matcher(collector, NULL);
}


