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
Analyzer_transform_text(Analyzer *self, CharBuf *text) {
    size_t token_len = CB_Get_Size(text);
    Token *seed = Token_new((char*)CB_Get_Ptr8(text), token_len, 0,
                            token_len, 1.0, 1);
    Inversion *starter = Inversion_new(seed);
    Inversion *retval  = Analyzer_Transform(self, starter);
    DECREF(seed);
    DECREF(starter);
    return retval;
}

VArray*
Analyzer_split(Analyzer *self, CharBuf *text) {
    Inversion  *inversion = Analyzer_Transform_Text(self, text);
    VArray     *out       = VA_new(0);
    Token      *token;

    while ((token = Inversion_Next(inversion)) != NULL) {
        VA_Push(out, (Obj*)CB_new_from_trusted_utf8(token->text, token->len));
    }

    DECREF(inversion);

    return out;
}


