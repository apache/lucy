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

#define C_LUCY_REGEXTOKENIZER
#define CHY_USE_SHORT_NAMES
#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES

#include "charmony.h"

#include <string.h>

#include "Lucy/Analysis/RegexTokenizer.h"
#include "Clownfish/CharBuf.h"
#include "Clownfish/Err.h"
#include "Clownfish/Util/Memory.h"
#include "Clownfish/Util/StringHelper.h"
#include "Lucy/Analysis/Token.h"
#include "Lucy/Analysis/Inversion.h"

#if defined(HAS_PCRE_H)

#include <pcre.h>

static uint32_t
S_count_code_points(const char *string, size_t len);

bool
RegexTokenizer_is_available(void) {
    return true;
}

RegexTokenizer*
RegexTokenizer_init(RegexTokenizer *self, const CharBuf *pattern) {
    Analyzer_init((Analyzer*)self);
    RegexTokenizerIVARS *const ivars = RegexTokenizer_IVARS(self);

    const char *pattern_ptr;
    if (pattern) {
        ivars->pattern = CB_Clone(pattern);
        pattern_ptr = (char*)CB_Get_Ptr8(ivars->pattern);
    }
    else {
        pattern_ptr = "\\w+(?:['\\x{2019}]\\w+)*";
        ivars->pattern
            = CB_new_from_trusted_utf8(pattern_ptr, strlen(pattern_ptr));
    }

    int options = PCRE_UTF8 | PCRE_NO_UTF8_CHECK;
#ifdef PCRE_BSR_UNICODE
    // Available since PCRE 7.4
    options |= PCRE_BSR_UNICODE;
#endif
#ifdef PCRE_NEWLINE_LF
    // Available since PCRE 6.7
    options |= PCRE_NEWLINE_LF;
#endif
    const char *err_ptr;
    int err_offset;
    pcre *re = pcre_compile(pattern_ptr, options, &err_ptr, &err_offset, NULL);
    if (!re) {
        THROW(ERR, "%s", err_ptr);
    }

    // TODO: Check whether pcre_study improves performance

    ivars->token_re = re;

    return self;
}

void
RegexTokenizer_set_token_re(RegexTokenizer *self, void *token_re) {
    UNUSED_VAR(self);
    UNUSED_VAR(token_re);
    THROW(ERR, "TODO");
}

void
RegexTokenizer_destroy(RegexTokenizer *self) {
    RegexTokenizerIVARS *const ivars = RegexTokenizer_IVARS(self);
    DECREF(ivars->pattern);
    pcre *re = (pcre*)ivars->token_re;
    if (re) {
        pcre_free(re);
    }
    SUPER_DESTROY(self, REGEXTOKENIZER);
}

void
RegexTokenizer_tokenize_str(RegexTokenizer *self,
                                 const char *string, size_t string_len,
                                 Inversion *inversion) {
    RegexTokenizerIVARS *const ivars = RegexTokenizer_IVARS(self);
    pcre      *re          = (pcre*)ivars->token_re;
    int        byte_offset = 0;
    uint32_t   cp_offset   = 0; // Code points
    int        options     = PCRE_NO_UTF8_CHECK;
    int        ovector[3];

    int return_code = pcre_exec(re, NULL, string, string_len, byte_offset,
                                options, ovector, 3);
    while (return_code >= 0) {
        const char *match     = string + ovector[0];
        size_t      match_len = ovector[1] - ovector[0];

        uint32_t cp_before  = S_count_code_points(string + byte_offset,
                                                  ovector[0] - byte_offset);
        uint32_t cp_start   = cp_offset + cp_before;
        uint32_t cp_matched = S_count_code_points(match, match_len);
        uint32_t cp_end     = cp_start + cp_matched;

        // Add a token to the new inversion.
        Token *token = Token_new(match, match_len, cp_start, cp_end, 1.0f, 1);
        Inversion_Append(inversion, token);

        byte_offset = ovector[1];
        cp_offset   = cp_end;
        return_code = pcre_exec(re, NULL, string, string_len, byte_offset,
                                options, ovector, 3);
    }

    if (return_code != PCRE_ERROR_NOMATCH) {
        THROW(ERR, "pcre_exec failed: %d", return_code);
    }
}

static uint32_t
S_count_code_points(const char *string, size_t len) {
    uint32_t num_code_points = 0;
    size_t i = 0;

    while (i < len) {
        i += StrHelp_UTF8_COUNT[(uint8_t)(string[i])];
        ++num_code_points;
    }

    if (i != len) {
        THROW(ERR, "Match between code point boundaries in '%s'", string);
    }

    return num_code_points;
}

#else // HAS_PCRE_H

bool
RegexTokenizer_is_available(void) {
    return false;
}

RegexTokenizer*
RegexTokenizer_init(RegexTokenizer *self, const CharBuf *pattern) {
    UNUSED_VAR(self);
    UNUSED_VAR(pattern);
    THROW(ERR,
          "RegexTokenizer is not available because Lucy was compiled"
          " without PCRE.");
    UNREACHABLE_RETURN(RegexTokenizer*);
}

void
RegexTokenizer_set_token_re(RegexTokenizer *self, void *token_re) {
    UNUSED_VAR(self);
    UNUSED_VAR(token_re);
    THROW(ERR,
          "RegexTokenizer is not available because Lucy was compiled"
          " without PCRE.");
}

void
RegexTokenizer_destroy(RegexTokenizer *self) {
    UNUSED_VAR(self);
    THROW(ERR,
          "RegexTokenizer is not available because Lucy was compiled"
          " without PCRE.");
}

void
RegexTokenizer_tokenize_str(RegexTokenizer *self, const char *string,
                            size_t string_len, Inversion *inversion) {
    UNUSED_VAR(self);
    UNUSED_VAR(string);
    UNUSED_VAR(string_len);
    UNUSED_VAR(inversion);
    THROW(ERR,
          "RegexTokenizer is not available because Lucy was compiled"
          " without PCRE.");
}

#endif // HAS_PCRE_H

