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

#define C_LUCY_SNOWBALLSTOPFILTER
#define C_LUCY_TOKEN
#include "Lucy/Util/ToolSet.h"
#include <ctype.h>

#include "Lucy/Analysis/SnowballStopFilter.h"
#include "Lucy/Analysis/Token.h"
#include "Lucy/Analysis/Inversion.h"

SnowballStopFilter*
SnowStop_new(const CharBuf *language, Hash *stoplist) {
    SnowballStopFilter *self = (SnowballStopFilter*)VTable_Make_Obj(SNOWBALLSTOPFILTER);
    return SnowStop_init(self, language, stoplist);
}

SnowballStopFilter*
SnowStop_init(SnowballStopFilter *self, const CharBuf *language,
              Hash *stoplist) {
    Analyzer_init((Analyzer*)self);

    if (stoplist) {
        if (language) { THROW(ERR, "Can't have both stoplist and language"); }
        self->stoplist = (Hash*)INCREF(stoplist);
    }
    else if (language) {
        self->stoplist = SnowStop_gen_stoplist(language);
        if (!self->stoplist) {
            THROW(ERR, "Can't get a stoplist for '%o'", language);
        }
    }
    else {
        THROW(ERR, "Either stoplist or language is required");
    }

    return self;
}

void
SnowStop_destroy(SnowballStopFilter *self) {
    DECREF(self->stoplist);
    SUPER_DESTROY(self, SNOWBALLSTOPFILTER);
}

Inversion*
SnowStop_transform(SnowballStopFilter *self, Inversion *inversion) {
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
SnowStop_equals(SnowballStopFilter *self, Obj *other) {
    SnowballStopFilter *const twin = (SnowballStopFilter*)other;
    if (twin == self)                         { return true; }
    if (!Obj_Is_A(other, SNOWBALLSTOPFILTER)) { return false; }
    if (!Hash_Equals(twin->stoplist, (Obj*)self->stoplist)) {
        return false;
    }
    return true;
}

Hash*
SnowStop_gen_stoplist(const CharBuf *language) {
    CharBuf *lang = CB_new(3);
    CB_Cat_Char(lang, tolower(CB_Code_Point_At(language, 0)));
    CB_Cat_Char(lang, tolower(CB_Code_Point_At(language, 1)));
    const uint8_t **words = NULL;
    if (CB_Equals_Str(lang, "da", 2))      { words = SnowStop_snow_da; }
    else if (CB_Equals_Str(lang, "de", 2)) { words = SnowStop_snow_de; }
    else if (CB_Equals_Str(lang, "en", 2)) { words = SnowStop_snow_en; }
    else if (CB_Equals_Str(lang, "es", 2)) { words = SnowStop_snow_es; }
    else if (CB_Equals_Str(lang, "fi", 2)) { words = SnowStop_snow_fi; }
    else if (CB_Equals_Str(lang, "fr", 2)) { words = SnowStop_snow_fr; }
    else if (CB_Equals_Str(lang, "hu", 2)) { words = SnowStop_snow_hu; }
    else if (CB_Equals_Str(lang, "it", 2)) { words = SnowStop_snow_it; }
    else if (CB_Equals_Str(lang, "nl", 2)) { words = SnowStop_snow_nl; }
    else if (CB_Equals_Str(lang, "no", 2)) { words = SnowStop_snow_no; }
    else if (CB_Equals_Str(lang, "pt", 2)) { words = SnowStop_snow_pt; }
    else if (CB_Equals_Str(lang, "ru", 2)) { words = SnowStop_snow_ru; }
    else if (CB_Equals_Str(lang, "sv", 2)) { words = SnowStop_snow_sv; }
    else {
        DECREF(lang);
        return NULL;
    }
    size_t num_stopwords = 0;
    for (uint32_t i = 0; words[i] != NULL; i++) { num_stopwords++; }
    NoCloneHash *stoplist = NoCloneHash_new(num_stopwords);
    for (uint32_t i = 0; words[i] != NULL; i++) {
        char *word = (char*)words[i];
        ViewCharBuf *stop = ViewCB_new_from_trusted_utf8(word, strlen(word));
        NoCloneHash_Store(stoplist, (Obj*)stop, INCREF(&EMPTY));
        DECREF(stop);
    }
    DECREF(lang);
    return (Hash*)stoplist;
}

/***************************************************************************/

NoCloneHash*
NoCloneHash_new(uint32_t capacity) {
    NoCloneHash *self = (NoCloneHash*)VTable_Make_Obj(NOCLONEHASH);
    return NoCloneHash_init(self, capacity);
}

NoCloneHash*
NoCloneHash_init(NoCloneHash *self, uint32_t capacity) {
    return (NoCloneHash*)Hash_init((Hash*)self, capacity);
}

Obj*
NoCloneHash_make_key(NoCloneHash *self, Obj *key, int32_t hash_sum) {
    UNUSED_VAR(self);
    UNUSED_VAR(hash_sum);
    return INCREF(key);
}

