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

#define C_LUCY_POSTING
#define C_LUCY_POSTINGWRITER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Index/Posting.h"
#include "KinoSearch/Index/DataWriter.h"
#include "KinoSearch/Index/PolyReader.h"
#include "KinoSearch/Index/Posting/RawPosting.h"
#include "KinoSearch/Index/Segment.h"
#include "KinoSearch/Index/Similarity.h"
#include "KinoSearch/Index/Snapshot.h"
#include "KinoSearch/Plan/Schema.h"
#include "KinoSearch/Store/InStream.h"

Posting*
Post_init(Posting *self)
{
    self->doc_id = 0;
    return self;
}

void
Post_set_doc_id(Posting *self, int32_t doc_id) { self->doc_id = doc_id; }
int32_t
Post_get_doc_id(Posting *self) { return self->doc_id; }

PostingWriter*
PostWriter_init(PostingWriter *self, Schema *schema, Snapshot *snapshot,
                Segment *segment, PolyReader *polyreader, int32_t field_num)
{
    DataWriter_init((DataWriter*)self, schema, snapshot, segment,
        polyreader);
    self->field_num = field_num;
    return self;
}


