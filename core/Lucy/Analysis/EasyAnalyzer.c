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
EasyAnalyzer_new(const CharBuf *language) {
    EasyAnalyzer *self = (EasyAnalyzer*)VTable_Make_Obj(EASYANALYZER);
    return EasyAnalyzer_init(self, language);
}

EasyAnalyzer*
EasyAnalyzer_init(EasyAnalyzer *self, const CharBuf *language) {
    Analyzer_init((Analyzer*)self);
    self->language   = CB_Clone(language);
    self->tokenizer  = StandardTokenizer_new();
    self->normalizer = Normalizer_new(NULL, true, false);
    self->stemmer    = SnowStemmer_new(language);
    return self;
}

void
EasyAnalyzer_destroy(EasyAnalyzer *self) {
    DECREF(self->language);
    DECREF(self->tokenizer);
    DECREF(self->normalizer);
    DECREF(self->stemmer);
    SUPER_DESTROY(self, EASYANALYZER);
}

Inversion*
EasyAnalyzer_transform(EasyAnalyzer *self, Inversion *inversion) {
    Inversion *inv1 = StandardTokenizer_Transform(self->tokenizer, inversion);
    Inversion *inv2 = Normalizer_Transform(self->normalizer, inv1);
    DECREF(inv1);
    inv1 = SnowStemmer_Transform(self->stemmer, inv2);
    DECREF(inv2);
    return inv1;
}

Inversion*
EasyAnalyzer_transform_text(EasyAnalyzer *self, CharBuf *text) {
    Inversion *inv1 = StandardTokenizer_Transform_Text(self->tokenizer, text);
    Inversion *inv2 = Normalizer_Transform(self->normalizer, inv1);
    DECREF(inv1);
    inv1 = SnowStemmer_Transform(self->stemmer, inv2);
    DECREF(inv2);
    return inv1;
}

Hash*
EasyAnalyzer_dump(EasyAnalyzer *self) {
    EasyAnalyzer_dump_t super_dump
        = (EasyAnalyzer_dump_t)SUPER_METHOD(EASYANALYZER, EasyAnalyzer, Dump);
    Hash *dump = super_dump(self);
    Hash_Store_Str(dump, "language", 8, (Obj*)CB_Clone(self->language));
    return dump;
}

EasyAnalyzer*
EasyAnalyzer_load(EasyAnalyzer *self, Obj *dump) {
    EasyAnalyzer_load_t super_load
        = (EasyAnalyzer_load_t)SUPER_METHOD(EASYANALYZER, EasyAnalyzer, Load);
    EasyAnalyzer *loaded = super_load(self, dump);
    Hash    *source = (Hash*)CERTIFY(dump, HASH);
    CharBuf *language
        = (CharBuf*)CERTIFY(Hash_Fetch_Str(source, "language", 8), CHARBUF);
    return EasyAnalyzer_init(loaded, language);
}

bool_t
EasyAnalyzer_equals(EasyAnalyzer *self, Obj *other) {
    EasyAnalyzer *const twin = (EasyAnalyzer*)other;
    if (twin == self)                                     { return true; }
    if (!Obj_Is_A(other, EASYANALYZER))                   { return false; }
    if (!CB_Equals(twin->language, (Obj*)self->language)) { return false; }
    return true;
}


