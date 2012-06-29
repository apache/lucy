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

#define C_LUCY_PARSERELEM
#include "Lucy/Util/ToolSet.h"
#include "Lucy/Search/QueryParser/ParserElem.h"

ParserElem*
ParserElem_new(uint32_t type, const char *text, size_t len) {
    ParserElem *self = (ParserElem*)VTable_Make_Obj(PARSERELEM);
    return ParserElem_init(self, type, text, len);
}

ParserElem*
ParserElem_init(ParserElem *self, uint32_t type, const char *text,
                 size_t len) {
    self->type = type;
    self->text = text ? CB_new_from_utf8(text, len) : NULL;
    return self;
}

void
ParserElem_destroy(ParserElem *self) {
    DECREF(self->text);
    SUPER_DESTROY(self, PARSERELEM);
}


