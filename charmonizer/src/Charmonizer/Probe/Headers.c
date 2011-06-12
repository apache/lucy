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

#define CHAZ_USE_SHORT_NAMES

#include "Charmonizer/Core/HeaderChecker.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Probe/Headers.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* Keep track of which headers have succeeded. */
static int keeper_count = 0;
#define MAX_KEEPER_COUNT 200
static const char *keepers[MAX_KEEPER_COUNT + 1] = { NULL };

/* Add a header to the keepers array.
 */
static void
S_keep(const char *header_name);

static size_t aff_buf_size = 0;
static char *aff_buf = NULL;

/* Transform "header.h" into "CHY_HAS_HEADER_H, storing the result in
 * [aff_buf].
 */
static void
S_encode_affirmation(const char *header_name);

#define NUM_C89_HEADERS 15
const char *c89_headers[] = {
    "assert.h",
    "ctype.h",
    "errno.h",
    "float.h",
    "limits.h",
    "locale.h",
    "math.h",
    "setjmp.h",
    "signal.h",
    "stdarg.h",
    "stddef.h",
    "stdio.h",
    "stdlib.h",
    "string.h",
    "time.h",
    NULL
};

#define NUM_POSIX_HEADERS 14
const char *posix_headers[] = {
    "cpio.h",
    "dirent.h",
    "fcntl.h",
    "grp.h",
    "pwd.h",
    "sys/stat.h",
    "sys/times.h",
    "sys/types.h",
    "sys/utsname.h",
    "sys/wait.h",
    "tar.h",
    "termios.h",
    "unistd.h",
    "utime.h",
    NULL
};

#define NUM_WIN_HEADERS 3
const char *win_headers[] = {
    "io.h",
    "windows.h",
    "process.h",
    NULL
};

chaz_bool_t
Headers_check(const char *header_name) {
    return HeadCheck_check_header(header_name);
}

void
Headers_run(void) {
    int i;
    chaz_bool_t has_posix = false;
    chaz_bool_t has_c89   = false;

    keeper_count = 0;

    ConfWriter_start_module("Headers");

    /* Try for all POSIX headers in one blast. */
    if (HeadCheck_check_many_headers((const char**)posix_headers)) {
        has_posix = true;
        ConfWriter_append_conf("#define CHY_HAS_POSIX\n");
        for (i = 0; posix_headers[i] != NULL; i++) {
            S_keep(posix_headers[i]);
        }
    }
    /* Test one-at-a-time. */
    else {
        for (i = 0; posix_headers[i] != NULL; i++) {
            if (HeadCheck_check_header(posix_headers[i])) {
                S_keep(posix_headers[i]);
            }
        }
    }

    /* Test for all c89 headers in one blast. */
    if (HeadCheck_check_many_headers((const char**)c89_headers)) {
        has_c89 = true;
        ConfWriter_append_conf("#define CHY_HAS_C89\n");
        ConfWriter_append_conf("#define CHY_HAS_C90\n");
        for (i = 0; c89_headers[i] != NULL; i++) {
            S_keep(c89_headers[i]);
        }
    }
    /* Test one-at-a-time. */
    else {
        for (i = 0; c89_headers[i] != NULL; i++) {
            if (HeadCheck_check_header(c89_headers[i])) {
                S_keep(c89_headers[i]);
            }
        }
    }

    /* Test for all Windows headers in one blast */
    if (HeadCheck_check_many_headers((const char**)win_headers)) {
        for (i = 0; win_headers[i] != NULL; i++) {
            S_keep(win_headers[i]);
        }
    }
    /* Test one-at-a-time. */
    else {
        for (i = 0; win_headers[i] != NULL; i++) {
            if (HeadCheck_check_header(win_headers[i])) {
                S_keep(win_headers[i]);
            }
        }
    }

    /* One-offs. */
    if (HeadCheck_check_header("pthread.h")) {
        S_keep("pthread.h");
    }

    /* Append the config with every header detected so far. */
    for (i = 0; keepers[i] != NULL; i++) {
        S_encode_affirmation(keepers[i]);
        ConfWriter_append_conf("#define CHY_%s\n", aff_buf);
    }

    /* Shorten. */
    ConfWriter_start_short_names();
    if (has_posix) {
        ConfWriter_shorten_macro("HAS_POSIX");
    }
    if (has_c89) {
        ConfWriter_shorten_macro("HAS_C89");
        ConfWriter_shorten_macro("HAS_C90");
    }
    for (i = 0; keepers[i] != NULL; i++) {
        S_encode_affirmation(keepers[i]);
        ConfWriter_shorten_macro(aff_buf);
    }
    ConfWriter_end_short_names();

    ConfWriter_end_module();
}

static void
S_keep(const char *header_name) {
    if (keeper_count >= MAX_KEEPER_COUNT) {
        Util_die("Too many keepers -- increase MAX_KEEPER_COUNT");
    }
    keepers[keeper_count++] = header_name;
    keepers[keeper_count]   = NULL;
}

static void
S_encode_affirmation(const char *header_name) {
    char *buf, *buf_end;
    size_t len = strlen(header_name) + sizeof("HAS_");

    /* Grow buffer and start off with "HAS_". */
    if (aff_buf_size < len + 1) {
        free(aff_buf);
        aff_buf_size = len + 1;
        aff_buf = (char*)malloc(aff_buf_size);
    }
    strcpy(aff_buf, "HAS_");

    /* Transform one char at a time. */
    for (buf = aff_buf + sizeof("HAS_") - 1, buf_end = aff_buf + len;
         buf < buf_end;
         header_name++, buf++
        ) {
        if (*header_name == '\0') {
            *buf = '\0';
            break;
        }
        else if (isalnum(*header_name)) {
            *buf = toupper(*header_name);
        }
        else {
            *buf = '_';
        }
    }
}



