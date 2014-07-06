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

#define C_LUCY_TERMINFO
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/TermInfo.h"
#include "Clownfish/Util/StringHelper.h"

TermInfo*
TInfo_new(int32_t doc_freq) {
    TermInfo *self = (TermInfo*)Class_Make_Obj(TERMINFO);
    return TInfo_init(self, doc_freq);
}

TermInfo*
TInfo_init(TermInfo *self, int32_t doc_freq) {
    TermInfoIVARS *const ivars = TInfo_IVARS(self);
    ivars->doc_freq      = doc_freq;
    ivars->post_filepos  = 0;
    ivars->skip_filepos  = 0;
    ivars->lex_filepos   = 0;
    return self;
}

TermInfo*
TInfo_Clone_IMP(TermInfo *self) {
    TermInfoIVARS *const ivars = TInfo_IVARS(self);
    TermInfo *twin = TInfo_new(ivars->doc_freq);
    TermInfoIVARS *const twin_ivars = TInfo_IVARS(twin);
    twin_ivars->post_filepos = ivars->post_filepos;
    twin_ivars->skip_filepos = ivars->skip_filepos;
    twin_ivars->lex_filepos  = ivars->lex_filepos;
    return twin;
}

int32_t
TInfo_Get_Doc_Freq_IMP(TermInfo *self) {
    return TInfo_IVARS(self)->doc_freq;
}

int64_t
TInfo_Get_Lex_FilePos_IMP(TermInfo *self) {
    return TInfo_IVARS(self)->lex_filepos;
}

int64_t
TInfo_Get_Post_FilePos_IMP(TermInfo *self) {
    return TInfo_IVARS(self)->post_filepos;
}

int64_t
TInfo_Get_Skip_FilePos_IMP(TermInfo *self) {
    return TInfo_IVARS(self)->skip_filepos;
}

void
TInfo_Set_Doc_Freq_IMP(TermInfo *self, int32_t doc_freq) {
    TInfo_IVARS(self)->doc_freq = doc_freq;
}

void
TInfo_Set_Lex_FilePos_IMP(TermInfo *self, int64_t filepos) {
    TInfo_IVARS(self)->lex_filepos = filepos;
}

void
TInfo_Set_Post_FilePos_IMP(TermInfo *self, int64_t filepos) {
    TInfo_IVARS(self)->post_filepos = filepos;
}

void
TInfo_Set_Skip_FilePos_IMP(TermInfo *self, int64_t filepos) {
    TInfo_IVARS(self)->skip_filepos = filepos;
}

// TODO: this should probably be some sort of Dump variant rather than
// To_String.
String*
TInfo_To_String_IMP(TermInfo *self) {
    TermInfoIVARS *const ivars = TInfo_IVARS(self);
    return Str_newf(
               "doc freq:      %i32\n"
               "post filepos:  %i64\n"
               "skip filepos:  %i64\n"
               "index filepos: %i64",
               ivars->doc_freq, ivars->post_filepos,
               ivars->skip_filepos, ivars->lex_filepos
           );
}

void
TInfo_Mimic_IMP(TermInfo *self, Obj *other) {
    CERTIFY(other, TERMINFO);
    TermInfoIVARS *const ivars = TInfo_IVARS(self);
    TermInfoIVARS *const ovars = TInfo_IVARS((TermInfo*)other);
    ivars->doc_freq     = ovars->doc_freq;
    ivars->post_filepos = ovars->post_filepos;
    ivars->skip_filepos = ovars->skip_filepos;
    ivars->lex_filepos  = ovars->lex_filepos;
}

void
TInfo_Reset_IMP(TermInfo *self) {
    TermInfoIVARS *const ivars = TInfo_IVARS(self);
    ivars->doc_freq      = 0;
    ivars->post_filepos  = 0;
    ivars->skip_filepos  = 0;
    ivars->lex_filepos   = 0;
}


