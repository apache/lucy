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

#define C_LUCY_SNOWBALLSTEMMER
#define C_LUCY_TOKEN
#include <ctype.h>
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Analysis/SnowballStemmer.h"
#include "Lucy/Analysis/Token.h"
#include "Lucy/Analysis/Inversion.h"

#include "libstemmer.h"

SnowballStemmer*
SnowStemmer_new(const CharBuf *language) {
    SnowballStemmer *self = (SnowballStemmer*)VTable_Make_Obj(SNOWBALLSTEMMER);
    return SnowStemmer_init(self, language);
}

SnowballStemmer*
SnowStemmer_init(SnowballStemmer *self, const CharBuf *language) {
    char lang_buf[3];
    Analyzer_init((Analyzer*)self);
    self->language = CB_Clone(language);

    // Get a Snowball stemmer.  Be case-insensitive.
    lang_buf[0] = tolower(CB_Code_Point_At(language, 0));
    lang_buf[1] = tolower(CB_Code_Point_At(language, 1));
    lang_buf[2] = '\0';
    self->snowstemmer = sb_stemmer_new(lang_buf, "UTF_8");
    if (!self->snowstemmer) {
        THROW(ERR, "Can't find a Snowball stemmer for %o", language);
    }

    return self;
}

void
SnowStemmer_destroy(SnowballStemmer *self) {
    if (self->snowstemmer) {
        sb_stemmer_delete((struct sb_stemmer*)self->snowstemmer);
    }
    DECREF(self->language);
    SUPER_DESTROY(self, SNOWBALLSTEMMER);
}

Inversion*
SnowStemmer_transform(SnowballStemmer *self, Inversion *inversion) {
    Token *token;
    struct sb_stemmer *const snowstemmer
        = (struct sb_stemmer*)self->snowstemmer;

    while (NULL != (token = Inversion_Next(inversion))) {
        const sb_symbol *stemmed_text 
            = sb_stemmer_stem(snowstemmer, (sb_symbol*)token->text, token->len);
        size_t len = sb_stemmer_length(snowstemmer);
        if (len > token->len) {
            FREEMEM(token->text);
            token->text = (char*)MALLOCATE(len + 1);
        }
        memcpy(token->text, stemmed_text, len + 1);
        token->len = len;
    }
    Inversion_Reset(inversion);
    return (Inversion*)INCREF(inversion);
}

Hash*
SnowStemmer_dump(SnowballStemmer *self) {
    SnowStemmer_dump_t super_dump
        = (SnowStemmer_dump_t)SUPER_METHOD(SNOWBALLSTEMMER, SnowStemmer, Dump);
    Hash *dump = super_dump(self);
    Hash_Store_Str(dump, "language", 8, (Obj*)CB_Clone(self->language));
    return dump;
}

SnowballStemmer*
SnowStemmer_load(SnowballStemmer *self, Obj *dump) {
    SnowStemmer_load_t super_load
        = (SnowStemmer_load_t)SUPER_METHOD(SNOWBALLSTEMMER, SnowStemmer, Load);
    SnowballStemmer *loaded = super_load(self, dump);
    Hash    *source = (Hash*)CERTIFY(dump, HASH);
    CharBuf *language 
        = (CharBuf*)CERTIFY(Hash_Fetch_Str(source, "language", 8), CHARBUF);
    return SnowStemmer_init(loaded, language);
}

bool_t
SnowStemmer_equals(SnowballStemmer *self, Obj *other) {
    SnowballStemmer *const twin = (SnowballStemmer*)other;
    if (twin == self)                                     { return true; }
    if (!Obj_Is_A(other, SNOWBALLSTEMMER))                { return false; }
    if (!CB_Equals(twin->language, (Obj*)self->language)) { return false; }
    return true;
}


