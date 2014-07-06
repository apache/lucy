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

#define C_LUCY_NOMATCHMATCHER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/NoMatchMatcher.h"
#include "Lucy/Index/IndexReader.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Plan/Schema.h"

NoMatchMatcher*
NoMatchMatcher_new() {
    NoMatchMatcher *self = (NoMatchMatcher*)Class_Make_Obj(NOMATCHMATCHER);
    return NoMatchMatcher_init(self);
}

NoMatchMatcher*
NoMatchMatcher_init(NoMatchMatcher *self) {
    return (NoMatchMatcher*)Matcher_init((Matcher*)self);
}

int32_t
NoMatchMatcher_Next_IMP(NoMatchMatcher* self) {
    UNUSED_VAR(self);
    return 0;
}

int32_t
NoMatchMatcher_Advance_IMP(NoMatchMatcher* self, int32_t target) {
    UNUSED_VAR(self);
    UNUSED_VAR(target);
    return 0;
}


