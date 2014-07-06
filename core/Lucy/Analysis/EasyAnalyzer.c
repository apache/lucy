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

#define C_LUCY_EASYANALYZER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Analysis/EasyAnalyzer.h"
#include "Lucy/Analysis/Inversion.h"
#include "Lucy/Analysis/StandardTokenizer.h"
#include "Lucy/Analysis/Normalizer.h"
#include "Lucy/Analysis/SnowballStemmer.h"

EasyAnalyzer*
EasyAnalyzer_new(String *language) {
    EasyAnalyzer *self = (EasyAnalyzer*)Class_Make_Obj(EASYANALYZER);
    return EasyAnalyzer_init(self, language);
}

EasyAnalyzer*
EasyAnalyzer_init(EasyAnalyzer *self, String *language) {
    Analyzer_init((Analyzer*)self);
    EasyAnalyzerIVARS *const ivars = EasyAnalyzer_IVARS(self);
    ivars->language   = Str_Clone(language);
    ivars->tokenizer  = StandardTokenizer_new();
    ivars->normalizer = Normalizer_new(NULL, true, false);
    ivars->stemmer    = SnowStemmer_new(language);
    return self;
}

void
EasyAnalyzer_Destroy_IMP(EasyAnalyzer *self) {
    EasyAnalyzerIVARS *const ivars = EasyAnalyzer_IVARS(self);
    DECREF(ivars->language);
    DECREF(ivars->tokenizer);
    DECREF(ivars->normalizer);
    DECREF(ivars->stemmer);
    SUPER_DESTROY(self, EASYANALYZER);
}

Inversion*
EasyAnalyzer_Transform_IMP(EasyAnalyzer *self, Inversion *inversion) {
    EasyAnalyzerIVARS *const ivars = EasyAnalyzer_IVARS(self);
    Inversion *inv1 = StandardTokenizer_Transform(ivars->tokenizer, inversion);
    Inversion *inv2 = Normalizer_Transform(ivars->normalizer, inv1);
    DECREF(inv1);
    inv1 = SnowStemmer_Transform(ivars->stemmer, inv2);
    DECREF(inv2);
    return inv1;
}

Inversion*
EasyAnalyzer_Transform_Text_IMP(EasyAnalyzer *self, String *text) {
    EasyAnalyzerIVARS *const ivars = EasyAnalyzer_IVARS(self);
    Inversion *inv1 = StandardTokenizer_Transform_Text(ivars->tokenizer, text);
    Inversion *inv2 = Normalizer_Transform(ivars->normalizer, inv1);
    DECREF(inv1);
    inv1 = SnowStemmer_Transform(ivars->stemmer, inv2);
    DECREF(inv2);
    return inv1;
}

Hash*
EasyAnalyzer_Dump_IMP(EasyAnalyzer *self) {
    EasyAnalyzerIVARS *const ivars = EasyAnalyzer_IVARS(self);
    EasyAnalyzer_Dump_t super_dump
        = SUPER_METHOD_PTR(EASYANALYZER, LUCY_EasyAnalyzer_Dump);
    Hash *dump = super_dump(self);
    Hash_Store_Utf8(dump, "language", 8, (Obj*)Str_Clone(ivars->language));
    return dump;
}

EasyAnalyzer*
EasyAnalyzer_Load_IMP(EasyAnalyzer *self, Obj *dump) {
    EasyAnalyzer_Load_t super_load
        = SUPER_METHOD_PTR(EASYANALYZER, LUCY_EasyAnalyzer_Load);
    EasyAnalyzer *loaded = super_load(self, dump);
    Hash    *source = (Hash*)CERTIFY(dump, HASH);
    String *language
        = (String*)CERTIFY(Hash_Fetch_Utf8(source, "language", 8), STRING);
    return EasyAnalyzer_init(loaded, language);
}

bool
EasyAnalyzer_Equals_IMP(EasyAnalyzer *self, Obj *other) {
    if ((EasyAnalyzer*)other == self)                       { return true; }
    if (!Obj_Is_A(other, EASYANALYZER))                     { return false; }
    EasyAnalyzerIVARS *const ivars = EasyAnalyzer_IVARS(self);
    EasyAnalyzerIVARS *const ovars = EasyAnalyzer_IVARS((EasyAnalyzer*)other);
    if (!Str_Equals(ovars->language, (Obj*)ivars->language)) { return false; }
    return true;
}


