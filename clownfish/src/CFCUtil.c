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
#include <stdlib.h>
#include <ctype.h>
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "CFCUtil.h"

void*
CFCUtil_make_perl_obj(void *ptr, const char *klass)
{
    SV *inner_obj = newSV(0);
    SvOBJECT_on(inner_obj);
    PL_sv_objcount++;
    SvUPGRADE(inner_obj, SVt_PVMG);
    sv_setiv(inner_obj, PTR2IV(ptr));

    // Connect class association.
    HV *stash = gv_stashpvn((char*)klass, strlen(klass), TRUE);
    SvSTASH_set(inner_obj, (HV*)SvREFCNT_inc(stash));

    return  inner_obj;
}

void
CFCUtil_null_check(const void *arg, const char *name, const char *file, int line)
{
    if (!arg) {
        croak("%s cannot be NULL at %s line %d", name, file, line);
    }
}

char*
CFCUtil_strdup(const char *string)
{
    if (!string) { return NULL; }
    return CFCUtil_strndup(string, strlen(string));
}

char*
CFCUtil_strndup(const char *string, size_t len)
{
    if (!string) { return NULL; }
    char *copy = (char*)malloc(len + 1);
    if (!copy) { croak("malloc failed"); }
    memcpy(copy, string, len);
    copy[len] = '\0';
    return copy;
}

void
CFCUtil_trim_whitespace(char *text)
{
    if (!text) {
        return;
    }

    // Find start.
    char *ptr = text;
    while (*ptr != '\0' && isspace(*ptr)) { ptr++; }

    // Find end.
    size_t orig_len = strlen(text);
    char *limit = text + orig_len;
    for ( ; limit > text; limit--) {
        if (!isspace(*(limit - 1))) { break; }
    }

    // Modify string in place and NULL-terminate.
    while (ptr < limit) {
        *text++ = *ptr++;
    }
    *text = '\0';
}

