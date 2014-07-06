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
#include "Lucy/Util/Freezer.h"

SnowballStopFilter*
SnowStop_new(String *language, Hash *stoplist) {
    SnowballStopFilter *self = (SnowballStopFilter*)Class_Make_Obj(SNOWBALLSTOPFILTER);
    return SnowStop_init(self, language, stoplist);
}

SnowballStopFilter*
SnowStop_init(SnowballStopFilter *self, String *language,
              Hash *stoplist) {
    Analyzer_init((Analyzer*)self);
    SnowballStopFilterIVARS *const ivars = SnowStop_IVARS(self);

    if (stoplist) {
        if (language) { THROW(ERR, "Can't have both stoplist and language"); }
        ivars->stoplist = (Hash*)INCREF(stoplist);
    }
    else if (language) {
        ivars->stoplist = SnowStop_gen_stoplist(language);
        if (!ivars->stoplist) {
            THROW(ERR, "Can't get a stoplist for '%o'", language);
        }
    }
    else {
        THROW(ERR, "Either stoplist or language is required");
    }

    return self;
}

void
SnowStop_Destroy_IMP(SnowballStopFilter *self) {
    SnowballStopFilterIVARS *const ivars = SnowStop_IVARS(self);
    DECREF(ivars->stoplist);
    SUPER_DESTROY(self, SNOWBALLSTOPFILTER);
}

Inversion*
SnowStop_Transform_IMP(SnowballStopFilter *self, Inversion *inversion) {
    Token *token;
    Inversion *new_inversion = Inversion_new(NULL);
    SnowballStopFilterIVARS *const ivars = SnowStop_IVARS(self);
    Hash *const stoplist  = ivars->stoplist;

    while (NULL != (token = Inversion_Next(inversion))) {
        TokenIVARS *const token_ivars = Token_IVARS(token);
        if (!Hash_Fetch_Utf8(stoplist, token_ivars->text, token_ivars->len)) {
            Inversion_Append(new_inversion, (Token*)INCREF(token));
        }
    }

    return new_inversion;
}

bool
SnowStop_Equals_IMP(SnowballStopFilter *self, Obj *other) {
    if ((SnowballStopFilter*)other == self)   { return true; }
    if (!Obj_Is_A(other, SNOWBALLSTOPFILTER)) { return false; }
    SnowballStopFilterIVARS *const ivars = SnowStop_IVARS(self);
    SnowballStopFilterIVARS *const ovars
        = SnowStop_IVARS((SnowballStopFilter*)other);
    if (!Hash_Equals(ivars->stoplist, (Obj*)ovars->stoplist)) {
        return false;
    }
    return true;
}

Obj*
SnowStop_Dump_IMP(SnowballStopFilter *self) {
    SnowballStopFilterIVARS *ivars = SnowStop_IVARS(self);
    SnowStop_Dump_t super_dump
        = SUPER_METHOD_PTR(SNOWBALLSTOPFILTER, LUCY_SnowStop_Dump);
    Hash *dump = (Hash*)CERTIFY(super_dump(self), HASH);
    if (ivars->stoplist) {
        Hash_Store_Utf8(dump, "stoplist", 8,
                        Freezer_dump((Obj*)ivars->stoplist));
    }
    return (Obj*)dump;
}

Obj*
SnowStop_Load_IMP(SnowballStopFilter *self, Obj *dump) {
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    SnowStop_Load_t super_load
        = SUPER_METHOD_PTR(SNOWBALLSTOPFILTER, LUCY_SnowStop_Load);
    SnowballStopFilter *loaded = (SnowballStopFilter*)super_load(self, dump);
    Obj *stoplist = Hash_Fetch_Utf8(source, "stoplist", 8);
    if (stoplist) {
        SnowStop_IVARS(loaded)->stoplist
            = (Hash*)CERTIFY(Freezer_load(stoplist), HASH);
    }
    return (Obj*)loaded;
}

Hash*
SnowStop_gen_stoplist(String *language) {
    char lang[2];
    lang[0] = tolower(Str_Code_Point_At(language, 0));
    lang[1] = tolower(Str_Code_Point_At(language, 1));
    const uint8_t **words = NULL;
    if (memcmp(lang, "da", 2) == 0)      { words = SnowStop_snow_da; }
    else if (memcmp(lang, "de", 2) == 0) { words = SnowStop_snow_de; }
    else if (memcmp(lang, "en", 2) == 0) { words = SnowStop_snow_en; }
    else if (memcmp(lang, "es", 2) == 0) { words = SnowStop_snow_es; }
    else if (memcmp(lang, "fi", 2) == 0) { words = SnowStop_snow_fi; }
    else if (memcmp(lang, "fr", 2) == 0) { words = SnowStop_snow_fr; }
    else if (memcmp(lang, "hu", 2) == 0) { words = SnowStop_snow_hu; }
    else if (memcmp(lang, "it", 2) == 0) { words = SnowStop_snow_it; }
    else if (memcmp(lang, "nl", 2) == 0) { words = SnowStop_snow_nl; }
    else if (memcmp(lang, "no", 2) == 0) { words = SnowStop_snow_no; }
    else if (memcmp(lang, "pt", 2) == 0) { words = SnowStop_snow_pt; }
    else if (memcmp(lang, "ru", 2) == 0) { words = SnowStop_snow_ru; }
    else if (memcmp(lang, "sv", 2) == 0) { words = SnowStop_snow_sv; }
    else {
        return NULL;
    }
    size_t num_stopwords = 0;
    for (uint32_t i = 0; words[i] != NULL; i++) { num_stopwords++; }
    Hash *stoplist = Hash_new(num_stopwords);
    for (uint32_t i = 0; words[i] != NULL; i++) {
        char *word = (char*)words[i];
        String *stop = Str_new_wrap_trusted_utf8(word, strlen(word));
        Hash_Store(stoplist, (Obj*)stop, (Obj*)CFISH_TRUE);
        DECREF(stop);
    }
    return (Hash*)stoplist;
}

