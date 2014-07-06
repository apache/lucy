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
#include "Lucy/Util/Freezer.h"

PolyAnalyzer*
PolyAnalyzer_new(String *language, VArray *analyzers) {
    PolyAnalyzer *self = (PolyAnalyzer*)Class_Make_Obj(POLYANALYZER);
    return PolyAnalyzer_init(self, language, analyzers);
}

PolyAnalyzer*
PolyAnalyzer_init(PolyAnalyzer *self, String *language,
                  VArray *analyzers) {
    Analyzer_init((Analyzer*)self);
    PolyAnalyzerIVARS *const ivars = PolyAnalyzer_IVARS(self);

    if (analyzers) {
        for (uint32_t i = 0, max = VA_Get_Size(analyzers); i < max; i++) {
            CERTIFY(VA_Fetch(analyzers, i), ANALYZER);
        }
        ivars->analyzers = (VArray*)INCREF(analyzers);
    }
    else if (language) {
        ivars->analyzers = VA_new(3);
        VA_Push(ivars->analyzers, (Obj*)CaseFolder_new());
        VA_Push(ivars->analyzers, (Obj*)RegexTokenizer_new(NULL));
        VA_Push(ivars->analyzers, (Obj*)SnowStemmer_new(language));
    }
    else {
        THROW(ERR, "Must specify either 'language' or 'analyzers'");
    }

    return self;
}

void
PolyAnalyzer_Destroy_IMP(PolyAnalyzer *self) {
    PolyAnalyzerIVARS *const ivars = PolyAnalyzer_IVARS(self);
    DECREF(ivars->analyzers);
    SUPER_DESTROY(self, POLYANALYZER);
}

VArray*
PolyAnalyzer_Get_Analyzers_IMP(PolyAnalyzer *self) {
    return PolyAnalyzer_IVARS(self)->analyzers;
}

Inversion*
PolyAnalyzer_Transform_IMP(PolyAnalyzer *self, Inversion *inversion) {
    VArray *const analyzers = PolyAnalyzer_IVARS(self)->analyzers;
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
PolyAnalyzer_Transform_Text_IMP(PolyAnalyzer *self, String *text) {
    VArray *const   analyzers     = PolyAnalyzer_IVARS(self)->analyzers;
    const uint32_t  num_analyzers = VA_Get_Size(analyzers);
    Inversion      *retval;

    if (num_analyzers == 0) {
        size_t      token_len = Str_Get_Size(text);
        const char *buf       = Str_Get_Ptr8(text);
        Token *seed = Token_new(buf, token_len, 0, token_len, 1.0f, 1);
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

bool
PolyAnalyzer_Equals_IMP(PolyAnalyzer *self, Obj *other) {
    if ((PolyAnalyzer*)other == self)                         { return true; }
    if (!Obj_Is_A(other, POLYANALYZER))                       { return false; }
    PolyAnalyzerIVARS *const ivars = PolyAnalyzer_IVARS(self);
    PolyAnalyzerIVARS *const ovars = PolyAnalyzer_IVARS((PolyAnalyzer*)other);
    if (!VA_Equals(ovars->analyzers, (Obj*)ivars->analyzers)) { return false; }
    return true;
}

Obj*
PolyAnalyzer_Dump_IMP(PolyAnalyzer *self) {
    PolyAnalyzerIVARS *const ivars = PolyAnalyzer_IVARS(self);
    PolyAnalyzer_Dump_t super_dump
        = SUPER_METHOD_PTR(POLYANALYZER, LUCY_PolyAnalyzer_Dump);
    Hash *dump = (Hash*)CERTIFY(super_dump(self), HASH);
    if (ivars->analyzers) {
        Hash_Store_Utf8(dump, "analyzers", 9,
                        Freezer_dump((Obj*)ivars->analyzers));
    }
    return (Obj*)dump;
}

PolyAnalyzer*
PolyAnalyzer_Load_IMP(PolyAnalyzer *self, Obj *dump) {
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    PolyAnalyzer_Load_t super_load 
        = SUPER_METHOD_PTR(POLYANALYZER, LUCY_PolyAnalyzer_Load);
    PolyAnalyzer *loaded = super_load(self, dump);
    VArray *analyzer_dumps
        = (VArray*)CERTIFY(Hash_Fetch_Utf8(source, "analyzers", 9), VARRAY);
    VArray *analyzers
        = (VArray*)CERTIFY(Freezer_load((Obj*)analyzer_dumps), VARRAY);
    PolyAnalyzer_init(loaded, NULL, analyzers);
    DECREF(analyzers);
    return loaded;
}


