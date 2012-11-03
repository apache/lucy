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
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Probe/Headers.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define CHAZ_HEADERS_MAX_KEEPERS 200

static struct {
    int keeper_count;
    const char *keepers[CHAZ_HEADERS_MAX_KEEPERS + 1];
} chaz_Headers = { 0, { NULL } };

/* Add a header to the keepers array.
 */
static void
S_keep(const char *header_name);

/* Transform "header.h" into "CHY_HAS_HEADER_H, storing the result into
 * `buffer`.
 */
static void
S_encode_affirmation(const char *header_name, char *buffer, size_t buf_size);

/* Probe for all C89 headers. */
static void
chaz_Headers_probe_c89(void);

/* Probe for all POSIX headers. */
static void
chaz_Headers_probe_posix(void);

/* Prove for selected Windows headers. */
static void
chaz_Headers_probe_win(void);

int
chaz_Headers_check(const char *header_name) {
    return chaz_HeadCheck_check_header(header_name);
}

void
chaz_Headers_run(void) {
    int i;

    chaz_ConfWriter_start_module("Headers");

    chaz_Headers_probe_posix();
    chaz_Headers_probe_c89();
    chaz_Headers_probe_win();

    /* One-offs. */
    if (chaz_HeadCheck_check_header("pthread.h")) {
        S_keep("pthread.h");
    }

    /* Append the config with every header detected so far. */
    for (i = 0; chaz_Headers.keepers[i] != NULL; i++) {
        char aff_buf[200];
        S_encode_affirmation(chaz_Headers.keepers[i], aff_buf, 200);
        chaz_ConfWriter_add_def(aff_buf, NULL);
    }

    chaz_ConfWriter_end_module();
}

static void
S_keep(const char *header_name) {
    if (chaz_Headers.keeper_count >= CHAZ_HEADERS_MAX_KEEPERS) {
        chaz_Util_die("Too many keepers -- increase MAX_KEEPER_COUNT");
    }
    chaz_Headers.keepers[chaz_Headers.keeper_count++] = header_name;
    chaz_Headers.keepers[chaz_Headers.keeper_count]   = NULL;
}

static void
S_encode_affirmation(const char *header_name, char *buffer, size_t buf_size) {
    char *buf, *buf_end;
    size_t len = strlen(header_name) + sizeof("HAS_");
    if (len + 1 > buf_size) {
        chaz_Util_die("Buffer too small: %lu", (unsigned long)buf_size);
    }

    /* Start off with "HAS_". */
    strcpy(buffer, "HAS_");

    /* Transform one char at a time. */
    for (buf = buffer + sizeof("HAS_") - 1, buf_end = buffer + len;
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

static void
chaz_Headers_probe_c89(void) {
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
    int i;

    /* Test for all c89 headers in one blast. */
    if (chaz_HeadCheck_check_many_headers((const char**)c89_headers)) {
        chaz_ConfWriter_add_def("HAS_C89", NULL);
        chaz_ConfWriter_add_def("HAS_C90", NULL);
        for (i = 0; c89_headers[i] != NULL; i++) {
            S_keep(c89_headers[i]);
        }
    }
    /* Test one-at-a-time. */
    else {
        for (i = 0; c89_headers[i] != NULL; i++) {
            if (chaz_HeadCheck_check_header(c89_headers[i])) {
                S_keep(c89_headers[i]);
            }
        }
    }
}

static void
chaz_Headers_probe_posix(void) {
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
    int i;

    /* Try for all POSIX headers in one blast. */
    if (chaz_HeadCheck_check_many_headers((const char**)posix_headers)) {
        chaz_ConfWriter_add_def("HAS_POSIX", NULL);
        for (i = 0; posix_headers[i] != NULL; i++) {
            S_keep(posix_headers[i]);
        }
    }
    /* Test one-at-a-time. */
    else {
        for (i = 0; posix_headers[i] != NULL; i++) {
            if (chaz_HeadCheck_check_header(posix_headers[i])) {
                S_keep(posix_headers[i]);
            }
        }
    }
}


static void
chaz_Headers_probe_win(void) {
    const char *win_headers[] = {
        "io.h",
        "windows.h",
        "process.h",
        NULL
    };
    int i;

    /* Test for all Windows headers in one blast */
    if (chaz_HeadCheck_check_many_headers((const char**)win_headers)) {
        for (i = 0; win_headers[i] != NULL; i++) {
            S_keep(win_headers[i]);
        }
    }
    /* Test one-at-a-time. */
    else {
        for (i = 0; win_headers[i] != NULL; i++) {
            if (chaz_HeadCheck_check_header(win_headers[i])) {
                S_keep(win_headers[i]);
            }
        }
    }
}

