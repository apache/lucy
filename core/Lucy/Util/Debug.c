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

#define C_LUCY_DEBUG
#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES

#include "Lucy/Util/Debug.h"

int32_t Debug_num_allocated = 0;
int32_t Debug_num_freed     = 0;
int32_t Debug_num_globals   = 0;

#if DEBUG_ENABLED

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

static char  env_cache_buf[256];
static char *env_cache            = NULL;
static char *env_cache_limit      = NULL;
static int   env_cache_is_current = 0;

// Cache the system call to getenv.
static void
S_cache_debug_env_var(char *override) {
    const char *debug_env = override ? override : getenv("DEBUG");
    if (debug_env != NULL) {
        size_t len = strlen(debug_env);
        if (len > sizeof(env_cache_buf) - 1) {
            len = sizeof(env_cache_buf) - 1;
        }
        strncpy(env_cache_buf, debug_env, len);
        env_cache       = env_cache_buf;
        env_cache_limit = env_cache + len;
    }
    env_cache_is_current = 1;
}

void
Debug_print_mess(const char *file, int line, const char *func,
                 const char *pat, ...) {
    va_list args;
    fprintf(stderr, "%s:%d %s(): ", file, line, func);
    va_start(args, pat);
    vfprintf(stderr, pat, args);
    va_end(args);
    fprintf(stderr, "\n");
}

int
Debug_debug_should_print(const char *path, const char *func) {
    if (!env_cache_is_current) {
        S_cache_debug_env_var(NULL);
    }

    if (!env_cache) {
        // Do not print if DEBUG environment var is not set.
        return 0;
    }
    else {
        const char *test, *next;
        const char *file = strrchr(path, '/');
        const int filename_len = file ? strlen(file) : 0;
        const int funcname_len = func ? strlen(func) : 0;

        // Use just file name if given path.
        if (file) { file++; }
        else      { file = path; }

        // Split criteria on commas. Bail when we run out of critieria.
        for (test = env_cache; test != NULL; test = next) {
            const char *last_char;

            // Skip whitespace.
            while (isspace(*test)) { test++; }
            if (test >= env_cache_limit) { return 0; }

            // Find end of criteria or end of string.
            next = strchr(test, ',');
            last_char = next ? next - 1 : env_cache_limit - 1;
            while (last_char > test && isspace(*last_char)) { last_char--; }

            if (*last_char == '*') {
                const int len = last_char - test;
                if (!strncmp(test, file, len)) { return 1; }
                if (!strncmp(test, func, len)) { return 1; }
            }
            else {
                if (!strncmp(test, file, filename_len)) { return 1; }
                if (!strncmp(test, func, funcname_len)) { return 1; }
            }
        }

        // No matches against the DEBUG environment var, so don't print.
        return 0;
    }
}

void
Debug_set_env_cache(char *override) {
    S_cache_debug_env_var(override);
}

#else // DEBUG

void
Debug_set_env_cache(char *override) {
    (void)override;
}

#endif // DEBUG

