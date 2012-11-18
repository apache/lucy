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
#include "Lucy/Search/QueryParser.h"

ParserElem*
ParserElem_new(uint32_t type, Obj *value) {
    ParserElem *self = (ParserElem*)VTable_Make_Obj(PARSERELEM);
    return ParserElem_init(self, type, value);
}

ParserElem*
ParserElem_init(ParserElem *self, uint32_t type, Obj *value) {
    self->type  = type;
    self->value = value;
    self->occur = LUCY_QPARSER_SHOULD;
    return self;
}

void
ParserElem_destroy(ParserElem *self) {
    DECREF(self->value);
    SUPER_DESTROY(self, PARSERELEM);
}

void
ParserElem_set_value(ParserElem *self, Obj *value) {
    INCREF(value);
    DECREF(self->value);
    self->value = value;
}

Obj*
ParserElem_as(ParserElem *self, VTable *metaclass) {
    if (self->value && Obj_Is_A(self->value, metaclass)) {
        return self->value;
    }
    return NULL;
}

uint32_t
ParserElem_get_type(ParserElem *self) {
    return self->type;
}

void
ParserElem_require(ParserElem *self) {
    switch (self->occur) {
        case LUCY_QPARSER_SHOULD:
            self->occur = LUCY_QPARSER_MUST;
            break;
        case LUCY_QPARSER_MUST_NOT:
        case LUCY_QPARSER_MUST:
            break;
        default:
            THROW(ERR, "Internal error in value of occur: %u32", self->occur);
    }
}

void
ParserElem_unrequire(ParserElem *self) {
    switch (self->occur) {
        case LUCY_QPARSER_MUST:
            self->occur = LUCY_QPARSER_SHOULD;
            break;
        case LUCY_QPARSER_MUST_NOT:
        case LUCY_QPARSER_SHOULD:
            break;
        default:
            THROW(ERR, "Internal error in value of occur: %u32", self->occur);
    }
}

void
ParserElem_negate(ParserElem *self) {
    switch (self->occur) {
        case LUCY_QPARSER_SHOULD:
        case LUCY_QPARSER_MUST:
            self->occur = LUCY_QPARSER_MUST_NOT;
            break;
        case LUCY_QPARSER_MUST_NOT:
            self->occur = LUCY_QPARSER_MUST; // Apply double negative.
            break;
        default:
            THROW(ERR, "Internal error in value of occur: %u32", self->occur);
    }
}

bool
ParserElem_optional(ParserElem *self) {
    return self->occur == LUCY_QPARSER_SHOULD;
}

bool
ParserElem_required(ParserElem *self) {
    return self->occur == LUCY_QPARSER_MUST;
}

bool
ParserElem_negated(ParserElem *self) {
    return self->occur == LUCY_QPARSER_MUST_NOT;
}

