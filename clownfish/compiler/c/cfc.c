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

#include "CFCBase.h"
#include "CFCBindCore.h"
#include "CFCC.h"
#include "CFCClass.h"
#include "CFCLexHeader.h"
#include "CFCHierarchy.h"
#include "CFCParcel.h"
#include "CFCUtil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct CFCArgs {
    char  *dest;
    int    num_source_dirs;
    char **source_dirs;
    int    num_include_dirs;
    char **include_dirs;
    char  *header_filename;
    char  *footer_filename;
};
typedef struct CFCArgs CFCArgs;

static int
S_parse_string_argument(const char *arg, const char *name, char **result) {
    size_t arg_len  = strlen(arg);
    size_t name_len = strlen(name);

    if (arg_len < name_len
        || memcmp(arg, name, name_len) != 0
        || arg[name_len] != '='
       ) {
        return 0;
    }

    if (*result != NULL) {
        fprintf(stderr, "Duplicate %s argument\n", name);
        exit(EXIT_FAILURE);
    }
    *result = CFCUtil_strdup(arg + name_len + 1);

    return 1;
}

static int
S_parse_string_array_argument(const char *arg, const char *name,
                              int *num_results, char ***results) {
    size_t   arg_len  = strlen(arg);
    size_t   name_len = strlen(name);
    int      new_num_results;
    char   **new_results;

    if (arg_len < name_len
        || memcmp(arg, name, name_len) != 0
        || arg[name_len] != '='
       ) {
        return 0;
    }

    new_num_results = *num_results + 1;
    new_results     = (char **)REALLOCATE(*results,
                              (new_num_results + 1) * sizeof(char *));
    new_results[new_num_results-1] = CFCUtil_strdup(arg + name_len + 1);
    new_results[new_num_results]   = NULL;
    *num_results = new_num_results;
    *results     = new_results;

    return 1;
}

/* Parse command line arguments. */
static void
S_parse_arguments(int argc, char **argv, CFCArgs *args) {
    int i;

    memset(args, 0, sizeof(CFCArgs));
    args->source_dirs     = (char **)MALLOCATE(sizeof(char *));
    args->source_dirs[0]  = NULL;
    args->include_dirs    = (char **)MALLOCATE(sizeof(char *));
    args->include_dirs[0] = NULL;

    for (i = 1; i < argc; i++) {
        char *arg = argv[i];

        if (S_parse_string_argument(arg, "--dest", &args->dest)) {
            continue;
        }
        if (S_parse_string_argument(arg, "--header", &args->header_filename)) {
            continue;
        }
        if (S_parse_string_argument(arg, "--footer", &args->footer_filename)) {
            continue;
        }
        if (S_parse_string_array_argument(arg, "--source",
                                          &args->num_source_dirs,
                                          &args->source_dirs)
           ) {
            continue;
        }
        if (S_parse_string_array_argument(arg, "--include",
                                          &args->num_include_dirs,
                                          &args->include_dirs)
           ) {
            continue;
        }

        fprintf(stderr, "Invalid argument '%s'\n", arg);
        exit(EXIT_FAILURE);
    }

    if (!args->dest) {
        fprintf(stderr, "Mandatory argument --dest missing\n");
        exit(EXIT_FAILURE);
    }
}

static void S_free_arguments(CFCArgs *args) {
    int i;

    if (args->dest)            { FREEMEM(args->dest); }
    if (args->header_filename) { FREEMEM(args->header_filename); }
    if (args->footer_filename) { FREEMEM(args->footer_filename); }

    for (i = 0; args->source_dirs[i]; ++i) {
        FREEMEM(args->source_dirs[i]);
    }
    FREEMEM(args->source_dirs);

    for (i = 0; args->include_dirs[i]; ++i) {
        FREEMEM(args->include_dirs[i]);
    }
    FREEMEM(args->include_dirs);
}

int
main(int argc, char **argv) {
    int           i;
    size_t        file_len;
    CFCArgs       args;
    CFCHierarchy *hierarchy;
    CFCBindCore  *core_binding;
    CFCC         *c_binding;
    char         *header = NULL;
    char         *footer = NULL;

    S_parse_arguments(argc, argv, &args);

    hierarchy = CFCHierarchy_new(args.dest);

    for (i = 0; args.source_dirs[i]; ++i) {
        CFCHierarchy_add_source_dir(hierarchy, args.source_dirs[i]);
    }
    for (i = 0; args.include_dirs[i]; ++i) {
        CFCHierarchy_add_source_dir(hierarchy, args.include_dirs[i]);
    }

    CFCHierarchy_build(hierarchy);

    if (args.header_filename) {
        header = CFCUtil_slurp_text(args.header_filename, &file_len);
    }
    else {
        header = CFCUtil_strdup("");
    }
    if (args.footer_filename) {
        footer = CFCUtil_slurp_text(args.footer_filename, &file_len);
    }
    else {
        footer = CFCUtil_strdup("");
    }

    core_binding = CFCBindCore_new(hierarchy, header, footer);
    CFCBindCore_write_all_modified(core_binding, 0);

    c_binding = CFCC_new(hierarchy, header, footer);
    CFCC_write_callbacks(c_binding);
    CFCC_write_hostdefs(c_binding);
    CFCC_write_man_pages(c_binding);

    CFCBase_decref((CFCBase*)c_binding);
    CFCBase_decref((CFCBase*)core_binding);
    CFCBase_decref((CFCBase*)hierarchy);
    FREEMEM(header);
    FREEMEM(footer);

    CFCClass_clear_registry();
    CFCParcel_reap_singletons();
    yylex_destroy();

    S_free_arguments(&args);

    return EXIT_SUCCESS;
}

