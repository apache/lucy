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
    ParserElem *self = (ParserElem*)Class_Make_Obj(PARSERELEM);
    return ParserElem_init(self, type, value);
}

ParserElem*
ParserElem_init(ParserElem *self, uint32_t type, Obj *value) {
    ParserElemIVARS *const ivars = ParserElem_IVARS(self);
    ivars->type  = type;
    ivars->value = value;
    ivars->occur = LUCY_QPARSER_SHOULD;
    return self;
}

void
ParserElem_Destroy_IMP(ParserElem *self) {
    ParserElemIVARS *const ivars = ParserElem_IVARS(self);
    DECREF(ivars->value);
    SUPER_DESTROY(self, PARSERELEM);
}

void
ParserElem_Set_Value_IMP(ParserElem *self, Obj *value) {
    ParserElemIVARS *const ivars = ParserElem_IVARS(self);
    Obj *new_value = INCREF(value);
    DECREF(ivars->value);
    ivars->value = new_value;
}

Obj*
ParserElem_As_IMP(ParserElem *self, Class *klass) {
    ParserElemIVARS *const ivars = ParserElem_IVARS(self);
    if (ivars->value && Obj_Is_A(ivars->value, klass)) {
        return ivars->value;
    }
    return NULL;
}

uint32_t
ParserElem_Get_Type_IMP(ParserElem *self) {
    return ParserElem_IVARS(self)->type;
}

void
ParserElem_Require_IMP(ParserElem *self) {
    ParserElemIVARS *const ivars = ParserElem_IVARS(self);
    switch (ivars->occur) {
        case LUCY_QPARSER_SHOULD:
            ivars->occur = LUCY_QPARSER_MUST;
            break;
        case LUCY_QPARSER_MUST_NOT:
        case LUCY_QPARSER_MUST:
            break;
        default:
            THROW(ERR, "Internal error in value of occur: %u32", ivars->occur);
    }
}

void
ParserElem_Unrequire_IMP(ParserElem *self) {
    ParserElemIVARS *const ivars = ParserElem_IVARS(self);
    switch (ivars->occur) {
        case LUCY_QPARSER_MUST:
            ivars->occur = LUCY_QPARSER_SHOULD;
            break;
        case LUCY_QPARSER_MUST_NOT:
        case LUCY_QPARSER_SHOULD:
            break;
        default:
            THROW(ERR, "Internal error in value of occur: %u32", ivars->occur);
    }
}

void
ParserElem_Negate_IMP(ParserElem *self) {
    ParserElemIVARS *const ivars = ParserElem_IVARS(self);
    switch (ivars->occur) {
        case LUCY_QPARSER_SHOULD:
        case LUCY_QPARSER_MUST:
            ivars->occur = LUCY_QPARSER_MUST_NOT;
            break;
        case LUCY_QPARSER_MUST_NOT:
            ivars->occur = LUCY_QPARSER_MUST; // Apply double negative.
            break;
        default:
            THROW(ERR, "Internal error in value of occur: %u32", ivars->occur);
    }
}

bool
ParserElem_Optional_IMP(ParserElem *self) {
    return ParserElem_IVARS(self)->occur == LUCY_QPARSER_SHOULD;
}

bool
ParserElem_Required_IMP(ParserElem *self) {
    return ParserElem_IVARS(self)->occur == LUCY_QPARSER_MUST;
}

bool
ParserElem_Negated_IMP(ParserElem *self) {
    return ParserElem_IVARS(self)->occur == LUCY_QPARSER_MUST_NOT;
}

