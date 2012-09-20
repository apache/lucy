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

#define C_LUCY_POSTINGLIST
#include <string.h>

#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/PostingList.h"
#include "Lucy/Index/Lexicon.h"
#include "Lucy/Index/Posting.h"
#include "Clownfish/Util/Memory.h"

PostingList*
PList_init(PostingList *self) {
    ABSTRACT_CLASS_CHECK(self, POSTINGLIST);
    return self;
}


