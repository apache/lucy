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

#define C_LUCY_NORMALIZER
#define C_LUCY_TOKEN
#include <ctype.h>
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Analysis/Normalizer.h"
#include "Lucy/Analysis/Token.h"
#include "Lucy/Analysis/Inversion.h"

#include "utf8proc.h"

#define INITIAL_BUFSIZE 63

Normalizer*
Normalizer_new(String *form, bool case_fold, bool strip_accents) {
    Normalizer *self = (Normalizer*)Class_Make_Obj(NORMALIZER);
    return Normalizer_init(self, form, case_fold, strip_accents);
}

Normalizer*
Normalizer_init(Normalizer *self, String *form, bool case_fold,
                bool strip_accents) {
    int options = UTF8PROC_STABLE;
    NormalizerIVARS *const ivars = Normalizer_IVARS(self);

    if (form == NULL
        || Str_Equals_Utf8(form, "NFKC", 4) || Str_Equals_Utf8(form, "nfkc", 4)
       ) {
        options |= UTF8PROC_COMPOSE | UTF8PROC_COMPAT;
    }
    else if (Str_Equals_Utf8(form, "NFC", 3) || Str_Equals_Utf8(form, "nfc", 3)) {
        options |= UTF8PROC_COMPOSE;
    }
    else if (Str_Equals_Utf8(form, "NFKD", 4) || Str_Equals_Utf8(form, "nfkd", 4)) {
        options |= UTF8PROC_DECOMPOSE | UTF8PROC_COMPAT;
    }
    else if (Str_Equals_Utf8(form, "NFD", 3) || Str_Equals_Utf8(form, "nfd", 3)) {
        options |= UTF8PROC_DECOMPOSE;
    }
    else {
        THROW(ERR, "Invalid normalization form %o", form);
    }

    if (case_fold)     { options |= UTF8PROC_CASEFOLD; }
    if (strip_accents) { options |= UTF8PROC_STRIPMARK; }

    ivars->options = options;

    return self;
}

Inversion*
Normalizer_Transform_IMP(Normalizer *self, Inversion *inversion) {
    // allocate additional space because utf8proc_reencode adds a
    // terminating null char
    int32_t static_buffer[INITIAL_BUFSIZE + 1];
    int32_t *buffer = static_buffer;
    ssize_t bufsize = INITIAL_BUFSIZE;
    Token *token;
    NormalizerIVARS *const ivars = Normalizer_IVARS(self);

    while (NULL != (token = Inversion_Next(inversion))) {
        TokenIVARS *const token_ivars = Token_IVARS(token);
        ssize_t len
            = utf8proc_decompose((uint8_t*)token_ivars->text,
                                 token_ivars->len, buffer, bufsize,
                                 ivars->options);

        if (len > bufsize) {
            // buffer too small, (re)allocate
            if (buffer != static_buffer) {
                FREEMEM(buffer);
            }
            // allocate additional INITIAL_BUFSIZE items
            bufsize = len + INITIAL_BUFSIZE;
            buffer = (int32_t*)MALLOCATE((bufsize + 1) * sizeof(int32_t));
            len = utf8proc_decompose((uint8_t*)token_ivars->text,
                                     token_ivars->len, buffer, bufsize,
                                     ivars->options);
        }
        if (len < 0) {
            continue;
        }

        len = utf8proc_reencode(buffer, len, ivars->options);

        if (len >= 0) {
            if (len > (ssize_t)token_ivars->len) {
                FREEMEM(token_ivars->text);
                token_ivars->text = (char*)MALLOCATE(len + 1);
            }
            memcpy(token_ivars->text, buffer, len + 1);
            token_ivars->len = len;
        }
    }

    if (buffer != static_buffer) {
        FREEMEM(buffer);
    }

    Inversion_Reset(inversion);
    return (Inversion*)INCREF(inversion);
}

Hash*
Normalizer_Dump_IMP(Normalizer *self) {
    Normalizer_Dump_t super_dump
        = SUPER_METHOD_PTR(NORMALIZER, LUCY_Normalizer_Dump);
    Hash *dump = super_dump(self);
    int options = Normalizer_IVARS(self)->options;

    String *form = options & UTF8PROC_COMPOSE ?
                    options & UTF8PROC_COMPAT ?
                    Str_new_from_trusted_utf8("NFKC", 4) :
                    Str_new_from_trusted_utf8("NFC", 3) :
                        options & UTF8PROC_COMPAT ?
                        Str_new_from_trusted_utf8("NFKD", 4) :
                        Str_new_from_trusted_utf8("NFD", 3);

    Hash_Store_Utf8(dump, "normalization_form", 18, (Obj*)form);

    BoolNum *case_fold = Bool_singleton(!!(options & UTF8PROC_CASEFOLD));
    Hash_Store_Utf8(dump, "case_fold", 9, (Obj*)case_fold);

    BoolNum *strip_accents = Bool_singleton(!!(options & UTF8PROC_STRIPMARK));
    Hash_Store_Utf8(dump, "strip_accents", 13, (Obj*)strip_accents);

    return dump;
}

Normalizer*
Normalizer_Load_IMP(Normalizer *self, Obj *dump) {
    Normalizer_Load_t super_load
        = SUPER_METHOD_PTR(NORMALIZER, LUCY_Normalizer_Load);
    Normalizer *loaded = super_load(self, dump);
    Hash    *source = (Hash*)CERTIFY(dump, HASH);

    Obj *obj = Hash_Fetch_Utf8(source, "normalization_form", 18);
    String *form = (String*)CERTIFY(obj, STRING);
    obj = Hash_Fetch_Utf8(source, "case_fold", 9);
    bool case_fold = Obj_To_Bool(CERTIFY(obj, OBJ));
    obj = Hash_Fetch_Utf8(source, "strip_accents", 13);
    bool strip_accents = Obj_To_Bool(CERTIFY(obj, OBJ));

    return Normalizer_init(loaded, form, case_fold, strip_accents);
}

bool
Normalizer_Equals_IMP(Normalizer *self, Obj *other) {
    if ((Normalizer*)other == self)       { return true; }
    if (!Obj_Is_A(other, NORMALIZER))     { return false; }
    NormalizerIVARS *const ivars = Normalizer_IVARS(self);
    NormalizerIVARS *const ovars = Normalizer_IVARS((Normalizer*)other);
    if (ovars->options != ivars->options) { return false; }
    return true;
}


