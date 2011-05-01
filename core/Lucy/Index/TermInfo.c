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
#include "Lucy/Util/StringHelper.h"

TermInfo*
TInfo_new(int32_t doc_freq) {
    TermInfo *self = (TermInfo*)VTable_Make_Obj(TERMINFO);
    return TInfo_init(self, doc_freq);
}

TermInfo*
TInfo_init(TermInfo *self, int32_t doc_freq) {
    self->doc_freq      = doc_freq;
    self->post_filepos  = 0;
    self->skip_filepos  = 0;
    self->lex_filepos   = 0;
    return self;
}

TermInfo*
TInfo_clone(TermInfo *self) {
    TermInfo *twin = TInfo_new(self->doc_freq);
    twin->post_filepos = self->post_filepos;
    twin->skip_filepos = self->skip_filepos;
    twin->lex_filepos  = self->lex_filepos;
    return twin;
}

int32_t
TInfo_get_doc_freq(TermInfo *self) {
    return self->doc_freq;
}

int64_t
TInfo_get_lex_filepos(TermInfo *self) {
    return self->lex_filepos;
}

int64_t
TInfo_get_post_filepos(TermInfo *self) {
    return self->post_filepos;
}

int64_t
TInfo_get_skip_filepos(TermInfo *self) {
    return self->skip_filepos;
}

void
TInfo_set_doc_freq(TermInfo *self, int32_t doc_freq) {
    self->doc_freq = doc_freq;
}

void
TInfo_set_lex_filepos(TermInfo *self, int64_t filepos) {
    self->lex_filepos = filepos;
}

void
TInfo_set_post_filepos(TermInfo *self, int64_t filepos) {
    self->post_filepos = filepos;
}

void
TInfo_set_skip_filepos(TermInfo *self, int64_t filepos) {
    self->skip_filepos = filepos;
}

// TODO: this should probably be some sort of Dump variant rather than
// To_String.
CharBuf*
TInfo_to_string(TermInfo *self) {
    return CB_newf(
               "doc freq:      %i32\n"
               "post filepos:  %i64\n"
               "skip filepos:  %i64\n"
               "index filepos: %i64",
               self->doc_freq, self->post_filepos,
               self->skip_filepos, self->lex_filepos
           );
}

void
TInfo_mimic(TermInfo *self, Obj *other) {
    TermInfo *twin = (TermInfo*)CERTIFY(other, TERMINFO);
    self->doc_freq     = twin->doc_freq;
    self->post_filepos = twin->post_filepos;
    self->skip_filepos = twin->skip_filepos;
    self->lex_filepos  = twin->lex_filepos;
}

void
TInfo_reset(TermInfo *self) {
    self->doc_freq      = 0;
    self->post_filepos  = 0;
    self->skip_filepos  = 0;
    self->lex_filepos   = 0;
}


