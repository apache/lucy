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

#define C_LUCY_DOC
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Document/Doc.h"

Doc*
Doc_new(void *fields, int32_t doc_id) {
    Doc *self = (Doc*)VTable_Make_Obj(DOC);
    return Doc_init(self, fields, doc_id);
}

void
Doc_set_doc_id(Doc *self, int32_t doc_id) {
    self->doc_id = doc_id;
}

int32_t
Doc_get_doc_id(Doc *self) {
    return self->doc_id;
}

void*
Doc_get_fields(Doc *self) {
    return self->fields;
}


