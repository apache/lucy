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
SnowStemmer_new(String *language) {
    SnowballStemmer *self = (SnowballStemmer*)Class_Make_Obj(SNOWBALLSTEMMER);
    return SnowStemmer_init(self, language);
}

SnowballStemmer*
SnowStemmer_init(SnowballStemmer *self, String *language) {
    char lang_buf[3];
    Analyzer_init((Analyzer*)self);
    SnowballStemmerIVARS *const ivars = SnowStemmer_IVARS(self);
    ivars->language = Str_Clone(language);

    // Get a Snowball stemmer.  Be case-insensitive.
    lang_buf[0] = tolower(Str_Code_Point_At(language, 0));
    lang_buf[1] = tolower(Str_Code_Point_At(language, 1));
    lang_buf[2] = '\0';
    ivars->snowstemmer = sb_stemmer_new(lang_buf, "UTF_8");
    if (!ivars->snowstemmer) {
        THROW(ERR, "Can't find a Snowball stemmer for %o", language);
    }

    return self;
}

void
SnowStemmer_Destroy_IMP(SnowballStemmer *self) {
    SnowballStemmerIVARS *const ivars = SnowStemmer_IVARS(self);
    if (ivars->snowstemmer) {
        sb_stemmer_delete((struct sb_stemmer*)ivars->snowstemmer);
    }
    DECREF(ivars->language);
    SUPER_DESTROY(self, SNOWBALLSTEMMER);
}

Inversion*
SnowStemmer_Transform_IMP(SnowballStemmer *self, Inversion *inversion) {
    Token *token;
    SnowballStemmerIVARS *const ivars = SnowStemmer_IVARS(self);
    struct sb_stemmer *const snowstemmer
        = (struct sb_stemmer*)ivars->snowstemmer;

    while (NULL != (token = Inversion_Next(inversion))) {
        TokenIVARS *const token_ivars = Token_IVARS(token);
        const sb_symbol *stemmed_text 
            = sb_stemmer_stem(snowstemmer, (sb_symbol*)token_ivars->text,
                              token_ivars->len);
        size_t len = sb_stemmer_length(snowstemmer);
        if (len > token_ivars->len) {
            FREEMEM(token_ivars->text);
            token_ivars->text = (char*)MALLOCATE(len + 1);
        }
        memcpy(token_ivars->text, stemmed_text, len + 1);
        token_ivars->len = len;
    }
    Inversion_Reset(inversion);
    return (Inversion*)INCREF(inversion);
}

Hash*
SnowStemmer_Dump_IMP(SnowballStemmer *self) {
    SnowballStemmerIVARS *const ivars = SnowStemmer_IVARS(self);
    SnowStemmer_Dump_t super_dump
        = SUPER_METHOD_PTR(SNOWBALLSTEMMER, LUCY_SnowStemmer_Dump);
    Hash *dump = super_dump(self);
    Hash_Store_Utf8(dump, "language", 8, (Obj*)Str_Clone(ivars->language));
    return dump;
}

SnowballStemmer*
SnowStemmer_Load_IMP(SnowballStemmer *self, Obj *dump) {
    SnowStemmer_Load_t super_load
        = SUPER_METHOD_PTR(SNOWBALLSTEMMER, LUCY_SnowStemmer_Load);
    SnowballStemmer *loaded = super_load(self, dump);
    Hash    *source = (Hash*)CERTIFY(dump, HASH);
    String *language 
        = (String*)CERTIFY(Hash_Fetch_Utf8(source, "language", 8), STRING);
    return SnowStemmer_init(loaded, language);
}

bool
SnowStemmer_Equals_IMP(SnowballStemmer *self, Obj *other) {
    if ((SnowballStemmer*)other == self)                    { return true; }
    if (!Obj_Is_A(other, SNOWBALLSTEMMER))                  { return false; }
    SnowballStemmerIVARS *ivars = SnowStemmer_IVARS(self);
    SnowballStemmerIVARS *ovars = SnowStemmer_IVARS((SnowballStemmer*)other);
    if (!Str_Equals(ovars->language, (Obj*)ivars->language)) { return false; }
    return true;
}


