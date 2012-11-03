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

#include "Charmonizer/Core/HeaderChecker.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include <string.h>
#include <stdlib.h>

typedef struct chaz_CHeader {
    const char  *name;
    int          exists;
} chaz_CHeader;

/* Keep a sorted, dynamically-sized array of names of all headers we've
 * checked for so far.
 */
static struct {
    int            cache_size;
    chaz_CHeader **header_cache;
} chaz_HeadCheck = { 0, NULL };

/* Comparison function to feed to qsort, bsearch, etc.
 */
static int
S_compare_headers(const void *vptr_a, const void *vptr_b);

/* Run a test compilation and return a new chaz_CHeader object encapsulating
 * the results.
 */
static chaz_CHeader*
S_discover_header(const char *header_name);

/* Extend the cache, add this chaz_CHeader object to it, and sort.
 */
static void
S_add_to_cache(chaz_CHeader *header);

/* Like add_to_cache, but takes a individual elements instead of a
 * chaz_CHeader* and checks if header exists in array first.
 */
static void
S_maybe_add_to_cache(const char *header_name, int exists);

void
chaz_HeadCheck_init(void) {
    chaz_CHeader *null_header = (chaz_CHeader*)malloc(sizeof(chaz_CHeader));

    /* Create terminating record for the dynamic array of chaz_CHeader
     * objects. */
    null_header->name   = NULL;
    null_header->exists = false;
    chaz_HeadCheck.header_cache    = (chaz_CHeader**)malloc(sizeof(void*));
    *(chaz_HeadCheck.header_cache) = null_header;
    chaz_HeadCheck.cache_size = 1;
}

int
chaz_HeadCheck_check_header(const char *header_name) {
    chaz_CHeader  *header;
    chaz_CHeader   key;
    chaz_CHeader  *fake = &key;
    chaz_CHeader **header_ptr;

    /* Fake up a key to feed to bsearch; see if the header's already there. */
    key.name = header_name;
    key.exists = false;
    header_ptr = (chaz_CHeader**)bsearch(&fake, chaz_HeadCheck.header_cache,
                                         chaz_HeadCheck.cache_size,
                                         sizeof(void*), S_compare_headers);

    /* If it's not there, go try a test compile. */
    if (header_ptr == NULL) {
        header = S_discover_header(header_name);
        S_add_to_cache(header);
    }
    else {
        header = *header_ptr;
    }

    return header->exists;
}

int
chaz_HeadCheck_check_many_headers(const char **header_names) {
    static const char test_code[] = "int main() { return 0; }\n";
    int success;
    int i;
    char *code_buf = chaz_Util_strdup("");
    size_t needed = sizeof(test_code) + 20;

    /* Build the source code string. */
    for (i = 0; header_names[i] != NULL; i++) {
        needed += strlen(header_names[i]);
        needed += sizeof("#include <>\n");
    }
    code_buf = (char*)malloc(needed);
    code_buf[0] = '\0';
    for (i = 0; header_names[i] != NULL; i++) {
        strcat(code_buf, "#include <");
        strcat(code_buf, header_names[i]);
        strcat(code_buf, ">\n");
    }
    strcat(code_buf, test_code);

    /* If the code compiles, bulk add all header names to the cache. */
    success = chaz_CC_test_compile(code_buf);
    if (success) {
        for (i = 0; header_names[i] != NULL; i++) {
            S_maybe_add_to_cache(header_names[i], true);
        }
    }

    free(code_buf);
    return success;
}

int
chaz_HeadCheck_contains_member(const char *struct_name, const char *member,
                               const char *includes) {
    static const char contains_code[] =
        CHAZ_QUOTE(  #include <stddef.h>                           )
        CHAZ_QUOTE(  %s                                            )
        CHAZ_QUOTE(  int main() { return offsetof(%s, %s); }       );
    long needed = sizeof(contains_code)
                  + strlen(struct_name)
                  + strlen(member)
                  + strlen(includes)
                  + 10;
    char *buf = (char*)malloc(needed);
    int retval;
    sprintf(buf, contains_code, includes, struct_name, member);
    retval = chaz_CC_test_compile(buf);
    free(buf);
    return retval;
}

static int
S_compare_headers(const void *vptr_a, const void *vptr_b) {
    chaz_CHeader *const *const a = (chaz_CHeader*const*)vptr_a;
    chaz_CHeader *const *const b = (chaz_CHeader*const*)vptr_b;

    /* (NULL is "greater than" any string.) */
    if ((*a)->name == NULL)      { return 1; }
    else if ((*b)->name == NULL) { return -1; }
    else                         { return strcmp((*a)->name, (*b)->name); }
}

static chaz_CHeader*
S_discover_header(const char *header_name) {
    static const char test_code[] = "int main() { return 0; }\n";
    chaz_CHeader* header = (chaz_CHeader*)malloc(sizeof(chaz_CHeader));
    size_t  needed = strlen(header_name) + sizeof(test_code) + 50;
    char *include_test = (char*)malloc(needed);

    /* Assign. */
    header->name = chaz_Util_strdup(header_name);

    /* See whether code that tries to pull in this header compiles. */
    sprintf(include_test, "#include <%s>\n%s", header_name, test_code);
    header->exists = chaz_CC_test_compile(include_test);

    free(include_test);
    return header;
}

static void
S_add_to_cache(chaz_CHeader *header) {
    size_t amount;

    /* Realloc array -- inefficient, but this isn't a bottleneck. */
    amount = ++chaz_HeadCheck.cache_size * sizeof(void*);
    chaz_HeadCheck.header_cache
        = (chaz_CHeader**)realloc(chaz_HeadCheck.header_cache, amount);
    chaz_HeadCheck.header_cache[chaz_HeadCheck.cache_size - 1] = header;

    /* Keep the list of headers sorted. */
    qsort(chaz_HeadCheck.header_cache, chaz_HeadCheck.cache_size,
          sizeof(*(chaz_HeadCheck.header_cache)), S_compare_headers);
}

static void
S_maybe_add_to_cache(const char *header_name, int exists) {
    chaz_CHeader *header;
    chaz_CHeader  key;
    chaz_CHeader *fake = &key;

    /* Fake up a key and bsearch for it. */
    key.name   = header_name;
    key.exists = exists;
    header = (chaz_CHeader*)bsearch(&fake, chaz_HeadCheck.header_cache,
                                    chaz_HeadCheck.cache_size, sizeof(void*),
                                    S_compare_headers);

    /* We've already done the test compile, so skip that step and add it. */
    if (header == NULL) {
        header = (chaz_CHeader*)malloc(sizeof(chaz_CHeader));
        header->name   = chaz_Util_strdup(header_name);
        header->exists = exists;
        S_add_to_cache(header);
    }
}


