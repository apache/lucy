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
RegexTokenizer_new(String *pattern) {
    RegexTokenizer *self = (RegexTokenizer*)Class_Make_Obj(REGEXTOKENIZER);
    return RegexTokenizer_init(self, pattern);
}

Inversion*
RegexTokenizer_Transform_IMP(RegexTokenizer *self, Inversion *inversion) {
    Inversion *new_inversion = Inversion_new(NULL);
    Token *token;

    while (NULL != (token = Inversion_Next(inversion))) {
        TokenIVARS *const token_ivars = Token_IVARS(token);
        RegexTokenizer_Tokenize_Utf8(self, token_ivars->text, token_ivars->len,
                                     new_inversion);
    }

    return new_inversion;
}

Inversion*
RegexTokenizer_Transform_Text_IMP(RegexTokenizer *self, String *text) {
    Inversion *new_inversion = Inversion_new(NULL);
    RegexTokenizer_Tokenize_Utf8(self, Str_Get_Ptr8(text),
                                 Str_Get_Size(text), new_inversion);
    return new_inversion;
}

Obj*
RegexTokenizer_Dump_IMP(RegexTokenizer *self) {
    RegexTokenizerIVARS *const ivars = RegexTokenizer_IVARS(self);
    RegexTokenizer_Dump_t super_dump
        = SUPER_METHOD_PTR(REGEXTOKENIZER, LUCY_RegexTokenizer_Dump);
    Hash *dump = (Hash*)CERTIFY(super_dump(self), HASH);
    Hash_Store_Utf8(dump, "pattern", 7, (Obj*)Str_Clone(ivars->pattern));
    return (Obj*)dump;
}

RegexTokenizer*
RegexTokenizer_Load_IMP(RegexTokenizer *self, Obj *dump) {
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    RegexTokenizer_Load_t super_load
        = SUPER_METHOD_PTR(REGEXTOKENIZER, LUCY_RegexTokenizer_Load);
    RegexTokenizer *loaded = super_load(self, dump);
    String *pattern 
        = (String*)CERTIFY(Hash_Fetch_Utf8(source, "pattern", 7), STRING);
    return RegexTokenizer_init(loaded, pattern);
}

bool
RegexTokenizer_Equals_IMP(RegexTokenizer *self, Obj *other) {
    if ((RegexTokenizer*)other == self)                   { return true; }
    if (!Obj_Is_A(other, REGEXTOKENIZER))                 { return false; }
    RegexTokenizerIVARS *ivars = RegexTokenizer_IVARS(self);
    RegexTokenizerIVARS *ovars = RegexTokenizer_IVARS((RegexTokenizer*)other);
    if (!Str_Equals(ivars->pattern, (Obj*)ovars->pattern)) { return false; }
    return true;
}


