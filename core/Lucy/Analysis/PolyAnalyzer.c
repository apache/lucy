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

#define C_LUCY_POLYANALYZER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Analysis/PolyAnalyzer.h"
#include "Lucy/Analysis/CaseFolder.h"
#include "Lucy/Analysis/Token.h"
#include "Lucy/Analysis/Inversion.h"
#include "Lucy/Analysis/SnowballStemmer.h"
#include "Lucy/Analysis/RegexTokenizer.h"

PolyAnalyzer*
PolyAnalyzer_new(const CharBuf *language, VArray *analyzers) {
    PolyAnalyzer *self = (PolyAnalyzer*)VTable_Make_Obj(POLYANALYZER);
    return PolyAnalyzer_init(self, language, analyzers);
}

PolyAnalyzer*
PolyAnalyzer_init(PolyAnalyzer *self, const CharBuf *language,
                  VArray *analyzers) {
    Analyzer_init((Analyzer*)self);
    if (analyzers) {
        for (uint32_t i = 0, max = VA_Get_Size(analyzers); i < max; i++) {
            CERTIFY(VA_Fetch(analyzers, i), ANALYZER);
        }
        self->analyzers = (VArray*)INCREF(analyzers);
    }
    else if (language) {
        self->analyzers = VA_new(3);
        VA_Push(self->analyzers, (Obj*)CaseFolder_new());
        VA_Push(self->analyzers, (Obj*)RegexTokenizer_new(NULL));
        VA_Push(self->analyzers, (Obj*)SnowStemmer_new(language));
    }
    else {
        THROW(ERR, "Must specify either 'language' or 'analyzers'");
    }

    return self;
}

void
PolyAnalyzer_destroy(PolyAnalyzer *self) {
    DECREF(self->analyzers);
    SUPER_DESTROY(self, POLYANALYZER);
}

VArray*
PolyAnalyzer_get_analyzers(PolyAnalyzer *self) {
    return self->analyzers;
}

Inversion*
PolyAnalyzer_transform(PolyAnalyzer *self, Inversion *inversion) {
    VArray *const analyzers = self->analyzers;
    (void)INCREF(inversion);

    // Iterate through each of the analyzers in order.
    for (uint32_t i = 0, max = VA_Get_Size(analyzers); i < max; i++) {
        Analyzer *analyzer = (Analyzer*)VA_Fetch(analyzers, i);
        Inversion *new_inversion = Analyzer_Transform(analyzer, inversion);
        DECREF(inversion);
        inversion = new_inversion;
    }

    return inversion;
}

Inversion*
PolyAnalyzer_transform_text(PolyAnalyzer *self, CharBuf *text) {
    VArray *const   analyzers     = self->analyzers;
    const uint32_t  num_analyzers = VA_Get_Size(analyzers);
    Inversion      *retval;

    if (num_analyzers == 0) {
        size_t  token_len = CB_Get_Size(text);
        char   *buf       = (char*)CB_Get_Ptr8(text);
        Token  *seed      = Token_new(buf, token_len, 0, token_len, 1.0f, 1);
        retval = Inversion_new(seed);
        DECREF(seed);
    }
    else {
        Analyzer *first_analyzer = (Analyzer*)VA_Fetch(analyzers, 0);
        retval = Analyzer_Transform_Text(first_analyzer, text);
        for (uint32_t i = 1; i < num_analyzers; i++) {
            Analyzer *analyzer = (Analyzer*)VA_Fetch(analyzers, i);
            Inversion *new_inversion = Analyzer_Transform(analyzer, retval);
            DECREF(retval);
            retval = new_inversion;
        }
    }

    return retval;
}

bool_t
PolyAnalyzer_equals(PolyAnalyzer *self, Obj *other) {
    PolyAnalyzer *const twin = (PolyAnalyzer*)other;
    if (twin == self)                                       { return true; }
    if (!Obj_Is_A(other, POLYANALYZER))                     { return false; }
    if (!VA_Equals(twin->analyzers, (Obj*)self->analyzers)) { return false; }
    return true;
}

PolyAnalyzer*
PolyAnalyzer_load(PolyAnalyzer *self, Obj *dump) {
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    PolyAnalyzer_load_t super_load = (PolyAnalyzer_load_t)SUPER_METHOD(
                                         POLYANALYZER, PolyAnalyzer, Load);
    PolyAnalyzer *loaded = super_load(self, dump);
    VArray *analyzer_dumps = (VArray*)CERTIFY(
                                 Hash_Fetch_Str(source, "analyzers", 9),
                                 VARRAY);
    VArray *analyzers = (VArray*)CERTIFY(
                            VA_Load(analyzer_dumps, (Obj*)analyzer_dumps),
                            VARRAY);
    PolyAnalyzer_init(loaded, NULL, analyzers);
    DECREF(analyzers);
    return loaded;
}


