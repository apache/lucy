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

#define C_LUCY_RAWLEXICON
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Index/RawLexicon.h"
#include "KinoSearch/Index/Posting/MatchPosting.h"
#include "KinoSearch/Index/TermStepper.h"
#include "KinoSearch/Index/TermInfo.h"
#include "KinoSearch/Plan/FieldType.h"
#include "KinoSearch/Plan/Schema.h"
#include "KinoSearch/Store/InStream.h"

RawLexicon*
RawLex_new(Schema *schema, const CharBuf *field, InStream *instream, 
           int64_t start, int64_t end)
{
    RawLexicon *self = (RawLexicon*)VTable_Make_Obj(RAWLEXICON);
    return RawLex_init(self, schema, field, instream, start, end);
}

RawLexicon*
RawLex_init(RawLexicon *self, Schema *schema, const CharBuf *field,
            InStream *instream, int64_t start, int64_t end)
{
    FieldType *type = Schema_Fetch_Type(schema, field);
    Lex_init((Lexicon*)self, field);
    
    // Assign 
    self->start = start;
    self->end   = end;
    self->len   = end - start;
    self->instream = (InStream*)INCREF(instream);

    // Get ready to begin. 
    InStream_Seek(self->instream, self->start);

    // Get steppers. 
    self->term_stepper  = FType_Make_Term_Stepper(type);
    self->tinfo_stepper = (TermStepper*)MatchTInfoStepper_new(schema);

    return self;
}

void
RawLex_destroy(RawLexicon *self)
{
    DECREF(self->instream);
    DECREF(self->term_stepper);
    DECREF(self->tinfo_stepper);
    SUPER_DESTROY(self, RAWLEXICON);
}

bool_t
RawLex_next(RawLexicon *self)
{
    if (InStream_Tell(self->instream) >= self->len) { return false; }
    TermStepper_Read_Delta(self->term_stepper, self->instream);
    TermStepper_Read_Delta(self->tinfo_stepper, self->instream);
    return true;
}

Obj*
RawLex_get_term(RawLexicon *self)
{
    return TermStepper_Get_Value(self->term_stepper);
}

int32_t
RawLex_doc_freq(RawLexicon *self)
{
    TermInfo *tinfo = (TermInfo*)TermStepper_Get_Value(self->tinfo_stepper);
    return tinfo ? TInfo_Get_Doc_Freq(tinfo) : 0;
}


