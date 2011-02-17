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

#include "CFCDocuComment.h"

struct CFCDocuComment {
    const char *description;
    const char *brief;
    const char *long_des;
    void *param_names;
    void *param_docs;
    const char *retval;
};

void
CFCDocuComment_strip(char *comment)
{
    CFCUtil_trim_whitespace(comment);

    size_t len = strlen(comment);
    char *scratch = (char*)malloc(len + 1);
    if (!scratch) {
        croak("malloc failed");
    }

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
            while (isspace(comment[i]) && comment[i] != '\n' && i < max) { 
                i++; 
            }
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
    free(scratch);
}

CFCDocuComment*
CFCDocuComment_new(const char *description, const char *brief, 
                   const char *long_description, void *param_names, 
                   void *param_docs, const char *retval)
{
    CFCDocuComment *self = (CFCDocuComment*)malloc(sizeof(CFCDocuComment));
    if (!self) { croak("malloc failed"); }
    return CFCDocuComment_init(self, description, brief, long_description,
        param_names, param_docs, retval);
}

CFCDocuComment*
CFCDocuComment_init(CFCDocuComment *self, const char *description, 
                    const char *brief, const char *long_description, 
                    void *param_names, void *param_docs, const char *retval)
{
    self->description = savepv(description);
    self->brief       = savepv(brief);
    self->long_des    = savepv(long_description);
    self->param_names = newSVsv((SV*)param_names);
    self->param_docs  = newSVsv((SV*)param_docs);
    self->retval      = retval ? savepv(retval) : NULL;
    return self;
}

void
CFCDocuComment_destroy(CFCDocuComment *self)
{
    Safefree(self->description);
    Safefree(self->brief);
    Safefree(self->long_des);
    SvREFCNT_dec((SV*)self->param_names);
    SvREFCNT_dec((SV*)self->param_docs);
    Safefree(self->retval);
    free(self);
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

void*
CFCDocuComment_get_param_names(CFCDocuComment *self)
{
    return self->param_names;
}

void*
CFCDocuComment_get_param_docs(CFCDocuComment *self)
{
    return self->param_docs;
}

const char*
CFCDocuComment_get_retval(CFCDocuComment *self)
{
    return self->retval;
}

