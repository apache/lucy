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

#define C_KINO_STOPALIZER
#define C_KINO_TOKEN
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Analysis/Stopalizer.h"
#include "KinoSearch/Analysis/Token.h"
#include "KinoSearch/Analysis/Inversion.h"

Stopalizer*
Stopalizer_new(const CharBuf *language, Hash *stoplist)
{
    Stopalizer *self = (Stopalizer*)VTable_Make_Obj(STOPALIZER);
    return Stopalizer_init(self, language, stoplist);
}

Stopalizer*
Stopalizer_init(Stopalizer *self, const CharBuf *language, Hash *stoplist)
{
    Analyzer_init((Analyzer*)self);

    if (stoplist) {
        if (language) { THROW(ERR, "Can't have both stoplist and language"); }
        self->stoplist = (Hash*)INCREF(stoplist);
    }
    else if (language) {
        self->stoplist = Stopalizer_gen_stoplist(language);
        if (!self->stoplist)
            THROW(ERR, "Can't get a stoplist for '%o'", language);
    }
    else {
        THROW(ERR, "Either stoplist or language is required");
    }

    return self;
}

void
Stopalizer_destroy(Stopalizer *self)
{
    DECREF(self->stoplist);
    SUPER_DESTROY(self, STOPALIZER);
}

Inversion*
Stopalizer_transform(Stopalizer *self, Inversion *inversion)
{
    Token *token;
    Inversion *new_inversion = Inversion_new(NULL);
    Hash *const stoplist  = self->stoplist;

    while (NULL != (token = Inversion_Next(inversion))) {
        if (!Hash_Fetch_Str(stoplist, token->text, token->len)) {
            Inversion_Append(new_inversion, (Token*)INCREF(token));
        }
    }

    return new_inversion;
}

bool_t
Stopalizer_equals(Stopalizer *self, Obj *other)
{
    Stopalizer *const evil_twin = (Stopalizer*)other;
    if (evil_twin == self) return true;
    if (!Obj_Is_A(other, STOPALIZER)) return false;
    if (!Hash_Equals(evil_twin->stoplist, (Obj*)self->stoplist)) {
        return false;
    }
    return true;
}


