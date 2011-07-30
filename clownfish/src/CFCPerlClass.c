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

#include <string.h>
#include <ctype.h>
#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCPerlClass.h"
#include "CFCUtil.h"
#include "CFCClass.h"
#include "CFCParcel.h"

struct CFCPerlClass {
    CFCBase base;
    CFCParcel *parcel;
    char *class_name;
    CFCClass *client;
    char *xs_code;
};

CFCPerlClass*
CFCPerlClass_new(CFCParcel *parcel, const char *class_name, CFCClass *client, 
                 const char *xs_code) {
    CFCPerlClass *self
        = (CFCPerlClass*)CFCBase_allocate(sizeof(CFCPerlClass),
                                          "Clownfish::Binding::Perl::Class");
    return CFCPerlClass_init(self, parcel, class_name, client, xs_code);
}

CFCPerlClass*
CFCPerlClass_init(CFCPerlClass *self, CFCParcel *parcel, const char *class_name,
                  CFCClass *client, const char *xs_code) {
    CFCUTIL_NULL_CHECK(parcel);
    CFCUTIL_NULL_CHECK(class_name);
    self->parcel = (CFCParcel*)CFCBase_incref((CFCBase*)parcel);
    self->client = (CFCClass*)CFCBase_incref((CFCBase*)client);
    self->class_name = CFCUtil_strdup(class_name);
    self->xs_code = xs_code ? CFCUtil_strdup(xs_code) : NULL;
    return self;
}

void
CFCPerlClass_destroy(CFCPerlClass *self) {
    CFCBase_decref((CFCBase*)self->parcel);
    CFCBase_decref((CFCBase*)self->client);
    FREEMEM(self->class_name);
    FREEMEM(self->xs_code);
    CFCBase_destroy((CFCBase*)self);
}

CFCClass*
CFCPerlClass_get_client(CFCPerlClass *self) {
    return self->client;
}

static char*
S_global_replace(const char *string, const char *match,
                 const char *replacement) {
    char *found = (char*)string;
    int   string_len      = strlen(string);
    int   match_len       = strlen(match);
    int   replacement_len = strlen(replacement);
    int   len_diff        = replacement_len - match_len;

    // Allocate space.
    unsigned count = 0;
    while (NULL != (found = strstr(found, match))) {
        count++;
        found += match_len;
    }
    int size = string_len + count * len_diff + 1;
    char *modified = (char*)MALLOCATE(size);
    modified[size - 1] = 0; // NULL-terminate.

    // Iterate through all matches.
    found = (char*)string;
    char *target = modified;
    size_t last_end = 0;
    if (count) {
        while (NULL != (found = strstr(found, match))) {
            size_t pos = found - string;
            size_t unchanged_len = pos - last_end;
            found += match_len;
            memcpy(target, string + last_end, unchanged_len);
            target += unchanged_len;
            last_end = pos + match_len;
            memcpy(target, replacement, replacement_len);
            target += replacement_len;
        }
    }
    size_t remaining = string_len - last_end;
    memcpy(target, string + string_len - remaining, remaining);

    return modified;
}

char*
CFCPerlClass_perlify_doc_text(CFCPerlClass *self, const char *source) {
    // Remove double-equals hack needed to fool perldoc, PAUSE, etc. :P
    // char *copy = S_global_replace(source, "==", "=");
    char *copy = CFCUtil_strdup(source);

    // Change <code>foo</code> to C<< foo >>.
    char *orig = copy;
    copy = S_global_replace(orig, "<code>", "C<< ");
    FREEMEM(orig);
    orig = copy;
    copy = S_global_replace(orig, "</code>", " >>");
    FREEMEM(orig);

    // Lowercase all method names: Open_In() => open_in()
    for (size_t i = 0, max = strlen(copy); i < max; i++) {
        if (isupper(copy[i])) {
            size_t mark = i;
            for (; i < max; i++) {
                char c = copy[i];
                if (!(isalpha(c) || c == '_')) {
                    if (memcmp(copy + i, "()", 2) == 0) {
                        for (size_t j = mark; j < i; j++) {
                            copy[j] = tolower(copy[j]);
                        }
                        i += 2; // go past parens.
                    }
                    break;
                }
            }
        }
    }

    // Change all instances of NULL to 'undef'
    orig = copy;
    copy = S_global_replace(orig, "NULL", "undef");
    FREEMEM(orig);

    // Change "Err_error" to "Lucy->error".
    orig = copy;
    copy = S_global_replace(orig, "Err_error", "Lucy->error");
    FREEMEM(orig);

    return copy;
}

const char*
CFCPerlClass_get_class_name(CFCPerlClass *self) {
    return self->class_name;
}

const char*
CFCPerlClass_get_xs_code(CFCPerlClass *self) {
    return self->xs_code;
}

