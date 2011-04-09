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

#include <stdlib.h>
#include <string.h>
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCDocuComment.h"
#include "CFCUtil.h"

struct CFCDocuComment {
    CFCBase base;
    char *description;
    char *brief;
    char *long_des;
    char **param_names;
    char **param_docs;
    char *retval;
};

/** Remove comment open, close, and left border from raw comment text.
 */
static void
S_strip(char *comment)
{
    size_t len = strlen(comment);
    char *scratch = (char*)MALLOCATE(len + 1);

    // Establish that comment text begins with "/**" and ends with "*/".
    if (   strstr(comment, "/**") != comment
        || strstr(comment, "*/") != (comment + len - 2)
    ) {
        croak("Malformed comment");
    }

    // Capture text minus beginning "/**", ending "*/", and left border.
    size_t i = 3;
    size_t max = len - 2;
    while ((isspace(comment[i]) || comment[i] == '*') && i < max) { 
        i++; 
    }
    size_t j = 0;
    for ( ; i < max; i++) {
        while (comment[i] == '\n' && i < max) {
            scratch[j++] = comment[i];
            i++;
            while (isspace(comment[i]) && comment[i] != '\n' && i < max) { 
                i++;
            }
            if (comment[i] == '*') { i++; }
            if (comment[i] == ' ') { i++; }
        }
        if (i < max) {
            scratch[j++] = comment[i];
        }
    }

    // Modify original string in place.
    for (i = 0; i < j; i++) {
        comment[i] = scratch[i];
    }
    comment[j] = '\0';

    // Clean up.
    FREEMEM(scratch);
}

CFCDocuComment*
CFCDocuComment_parse(const char *raw_text)
{
    char *text = CFCUtil_strdup(raw_text);
    CFCDocuComment *self = (CFCDocuComment*)CFCBase_allocate(
        sizeof(CFCDocuComment), "Clownfish::DocuComment");

    // Strip whitespace, comment open, close, and left border.
    CFCUtil_trim_whitespace(text);
    S_strip(text);

    // Extract the brief description.
    {
        char *ptr = text;
        size_t len = strlen(text);
        char *limit = strchr(ptr, '@');
        if (!limit) {
            limit = text + len;
        }
        while (ptr < limit) {
            if (   *ptr == '.' 
                && ((ptr == limit - 1) || isspace(*(ptr + 1)))
            ) {
                ptr++;
                size_t brief_len = ptr - text;
                self->brief = CFCUtil_strdup(text);
                self->brief[brief_len] = '\0';
                break;
            }
            ptr++;
        }
    }
    if (!self->brief) {
        self->brief = CFCUtil_strdup("");
    }

    // Extract @param directives.
    size_t num_params = 0;
    self->param_names = (char**)CALLOCATE(num_params + 1, sizeof(char*));
    self->param_docs  = (char**)CALLOCATE(num_params + 1, sizeof(char*));
    {
        char *candidate = strstr(text, "@param");
        size_t len = strlen(text);
        char *limit = text + len;
        while (candidate) {
            // Extract param name.
            char *ptr = candidate + sizeof("@param") - 1;
            if (!isspace(*ptr) || ptr > limit) {
                croak("Malformed @param directive in '%s'", raw_text);
            }
            while (isspace(*ptr) && ptr < limit) { ptr++; }
            char *param_name = ptr;
            while ((isalnum(*ptr) || *ptr == '_') && ptr < limit) { ptr++; }
            size_t param_name_len = ptr - param_name;
            if (!param_name_len) {
                croak("Malformed @param directive in '%s'", raw_text);
            }

            // Extract param description.
            while (isspace(*ptr) && ptr < limit) { ptr++; }
            char *param_doc = ptr;
            while (ptr < limit) {
                if (*ptr == '@') { break; }
                else if (*ptr == '\n' && ptr < limit) {
                    ptr++;
                    while (ptr < limit && *ptr != '\n' && isspace(*ptr)) {
                        ptr++;
                    }
                    if (*ptr == '\n' || *ptr == '@') { break; }
                }
                else {
                    ptr++;
                }
            }
            size_t param_doc_len = ptr - param_doc;

            num_params++;
            size_t size = (num_params + 1) * sizeof(char*);
            self->param_names = (char**)REALLOCATE(self->param_names, size);
            self->param_docs  = (char**)REALLOCATE(self->param_docs, size);
            self->param_names[num_params - 1] 
                = CFCUtil_strndup(param_name, param_name_len);
            self->param_docs[num_params - 1] 
                = CFCUtil_strndup(param_doc, param_doc_len);
            CFCUtil_trim_whitespace(self->param_names[num_params - 1]);
            CFCUtil_trim_whitespace(self->param_docs[num_params - 1]);
            self->param_names[num_params] = NULL;
            self->param_docs[num_params]  = NULL;
            
            if (ptr == limit) {
                break;
            }
            else if (ptr > limit) {
                croak("Overran end of string while parsing '%s'", raw_text);
            }
            candidate = strstr(ptr, "@param");
        }
    }

    // Extract full description.
    self->description = CFCUtil_strdup(text);
    {
        char *terminus = strstr(self->description, "@param");
        if (!terminus) {
            terminus = strstr(self->description, "@return");
        }
        if (terminus) {
            *terminus = '\0';
        }
    }
    CFCUtil_trim_whitespace(self->description);

    // Extract long description.
    self->long_des = CFCUtil_strdup(self->description + strlen(self->brief));
    CFCUtil_trim_whitespace(self->long_des);

    // Extract @return directive.
    char *maybe_retval = strstr(text, "@return ");
    if (maybe_retval) {
        self->retval = CFCUtil_strdup(maybe_retval + sizeof("@return ") - 1);
        char *terminus = strchr(self->retval, '@');
        if (terminus) {
            *terminus = '\0';
        }
        CFCUtil_trim_whitespace(self->retval);
    }

    FREEMEM(text);
    return self;
}

void
CFCDocuComment_destroy(CFCDocuComment *self)
{
    size_t i;
    if (self->param_names) {
        for (i = 0; self->param_names[i] != NULL; i++) {
            FREEMEM(self->param_names[i]);
        }
        FREEMEM(self->param_names);
    }
    if (self->param_docs) {
        for (i = 0; self->param_docs[i] != NULL; i++) {
            FREEMEM(self->param_docs[i]);
        }
        FREEMEM(self->param_docs);
    }
    FREEMEM(self->description);
    FREEMEM(self->brief);
    FREEMEM(self->long_des);
    FREEMEM(self->retval);
    CFCBase_destroy((CFCBase*)self);
}

const char*
CFCDocuComment_get_description(CFCDocuComment *self)
{
    return self->description;
}

const char*
CFCDocuComment_get_brief(CFCDocuComment *self)
{
    return self->brief;
}

const char*
CFCDocuComment_get_long(CFCDocuComment *self)
{
    return self->long_des;
}

const char**
CFCDocuComment_get_param_names(CFCDocuComment *self)
{
    return (const char**)self->param_names;
}

const char**
CFCDocuComment_get_param_docs(CFCDocuComment *self)
{
    return (const char**)self->param_docs;
}

const char*
CFCDocuComment_get_retval(CFCDocuComment *self)
{
    return self->retval;
}

