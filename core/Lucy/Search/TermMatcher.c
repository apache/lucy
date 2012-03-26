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

#define C_LUCY_TERMMATCHER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/TermMatcher.h"
#include "Lucy/Index/Posting.h"
#include "Lucy/Index/PostingList.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Search/Compiler.h"

TermMatcher*
TermMatcher_init(TermMatcher *self, Similarity *similarity, PostingList *plist,
                 Compiler *compiler) {
    Matcher_init((Matcher*)self);

    // Assign.
    self->sim           = (Similarity*)INCREF(similarity);
    self->plist         = (PostingList*)INCREF(plist);
    self->compiler      = (Compiler*)INCREF(compiler);
    self->weight        = Compiler_Get_Weight(compiler);

    // Init.
    self->posting        = NULL;

    return self;
}

void
TermMatcher_destroy(TermMatcher *self) {
    DECREF(self->sim);
    DECREF(self->plist);
    DECREF(self->compiler);
    SUPER_DESTROY(self, TERMMATCHER);
}

int32_t
TermMatcher_next(TermMatcher* self) {
    PostingList *const plist = self->plist;
    if (plist) {
        int32_t doc_id = PList_Next(plist);
        if (doc_id) {
            self->posting = PList_Get_Posting(plist);
            return doc_id;
        }
        else {
            // Reclaim resources a little early.
            DECREF(plist);
            self->plist = NULL;
            return 0;
        }
    }
    return 0;
}

int32_t
TermMatcher_advance(TermMatcher *self, int32_t target) {
    PostingList *const plist = self->plist;
    if (plist) {
        int32_t doc_id = PList_Advance(plist, target);
        if (doc_id) {
            self->posting = PList_Get_Posting(plist);
            return doc_id;
        }
        else {
            // Reclaim resources a little early.
            DECREF(plist);
            self->plist = NULL;
            return 0;
        }
    }
    return 0;
}

int32_t
TermMatcher_get_doc_id(TermMatcher* self) {
    return Post_Get_Doc_ID(self->posting);
}


