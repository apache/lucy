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
#include "Lucy/Analysis/PolyAnalyzer.h"
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
    VArray *analyzers = VA_new(3);
    VA_Push(analyzers, (Obj*)StandardTokenizer_new());
    VA_Push(analyzers, (Obj*)Normalizer_new(NULL, true, false));
    VA_Push(analyzers, (Obj*)SnowStemmer_new(language));
    PolyAnalyzer_init((PolyAnalyzer*)self, NULL, analyzers);
    DECREF(analyzers);
    return self;
}


