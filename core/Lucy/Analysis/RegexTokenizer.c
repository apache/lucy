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

#define C_LUCY_REGEXTOKENIZER
#define C_LUCY_TOKEN
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Analysis/RegexTokenizer.h"
#include "Lucy/Analysis/Token.h"
#include "Lucy/Analysis/Inversion.h"

RegexTokenizer*
RegexTokenizer_new(const CharBuf *pattern) {
    RegexTokenizer *self = (RegexTokenizer*)VTable_Make_Obj(REGEXTOKENIZER);
    return RegexTokenizer_init(self, pattern);
}

Inversion*
RegexTokenizer_transform(RegexTokenizer *self, Inversion *inversion) {
    Inversion *new_inversion = Inversion_new(NULL);
    Token *token;

    while (NULL != (token = Inversion_Next(inversion))) {
        RegexTokenizer_Tokenize_Str(self, token->text, token->len,
                                    new_inversion);
    }

    return new_inversion;
}

Inversion*
RegexTokenizer_transform_text(RegexTokenizer *self, CharBuf *text) {
    Inversion *new_inversion = Inversion_new(NULL);
    RegexTokenizer_Tokenize_Str(self, (char*)CB_Get_Ptr8(text),
                                CB_Get_Size(text), new_inversion);
    return new_inversion;
}

Obj*
RegexTokenizer_dump(RegexTokenizer *self) {
    RegexTokenizer_dump_t super_dump
        = (RegexTokenizer_dump_t)SUPER_METHOD(REGEXTOKENIZER, RegexTokenizer, Dump);
    Hash *dump = (Hash*)CERTIFY(super_dump(self), HASH);
    Hash_Store_Str(dump, "pattern", 7, CB_Dump(self->pattern));
    return (Obj*)dump;
}

RegexTokenizer*
RegexTokenizer_load(RegexTokenizer *self, Obj *dump) {
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    RegexTokenizer_load_t super_load
        = (RegexTokenizer_load_t)SUPER_METHOD(REGEXTOKENIZER, RegexTokenizer, Load);
    RegexTokenizer *loaded = super_load(self, dump);
    CharBuf *pattern 
        = (CharBuf*)CERTIFY(Hash_Fetch_Str(source, "pattern", 7), CHARBUF);
    return RegexTokenizer_init(loaded, pattern);
}

bool_t
RegexTokenizer_equals(RegexTokenizer *self, Obj *other) {
    RegexTokenizer *const twin = (RegexTokenizer*)other;
    if (twin == self)                                   { return true; }
    if (!Obj_Is_A(other, REGEXTOKENIZER))               { return false; }
    if (!CB_Equals(twin->pattern, (Obj*)self->pattern)) { return false; }
    return true;
}


