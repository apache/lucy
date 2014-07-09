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

#define C_LUCY_ANALYZER
#define C_LUCY_TOKEN
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Analysis/Analyzer.h"
#include "Lucy/Analysis/Token.h"
#include "Lucy/Analysis/Inversion.h"

Analyzer*
Analyzer_init(Analyzer *self) {
    ABSTRACT_CLASS_CHECK(self, ANALYZER);
    return self;
}

Inversion*
Analyzer_Transform_Text_IMP(Analyzer *self, String *text) {
    size_t token_len = Str_Get_Size(text);
    Token *seed = Token_new(Str_Get_Ptr8(text), token_len, 0,
                            token_len, 1.0, 1);
    Inversion *starter = Inversion_new(seed);
    Inversion *retval  = Analyzer_Transform(self, starter);
    DECREF(seed);
    DECREF(starter);
    return retval;
}

VArray*
Analyzer_Split_IMP(Analyzer *self, String *text) {
    Inversion  *inversion = Analyzer_Transform_Text(self, text);
    VArray     *out       = VA_new(0);
    Token      *token;

    while ((token = Inversion_Next(inversion)) != NULL) {
        TokenIVARS *const token_ivars = Token_IVARS(token);
        String *string
            = Str_new_from_trusted_utf8(token_ivars->text, token_ivars->len);
        VA_Push(out, (Obj*)string);
    }

    DECREF(inversion);

    return out;
}

Obj*
Analyzer_Dump_IMP(Analyzer *self) {
    Hash *dump = Hash_new(0);
    Hash_Store_Utf8(dump, "_class", 6,
                    (Obj*)Str_Clone(Obj_Get_Class_Name((Obj*)self)));
    return (Obj*)dump;
}

Obj*
Analyzer_Load_IMP(Analyzer *self, Obj *dump) {
    UNUSED_VAR(self);
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    String *class_name
        = (String*)CERTIFY(Hash_Fetch_Utf8(source, "_class", 6), STRING);
    Class *klass = Class_singleton(class_name, NULL);
    Analyzer *loaded = (Analyzer*)Class_Make_Obj(klass);
    return (Obj*)loaded;
}

