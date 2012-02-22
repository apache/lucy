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
#include <stdio.h>
#include <ctype.h>
#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCPerl.h"
#include "CFCParcel.h"
#include "CFCClass.h"
#include "CFCHierarchy.h"
#include "CFCUtil.h"
#include "CFCPerlClass.h"

struct CFCPerl {
    CFCBase base;
    CFCParcel *parcel;
    CFCHierarchy *hierarchy;
    char *lib_dir;
    char *boot_class;
    char *header;
    char *footer;
    char *xs_path;
    char *pm_path;
    char *boot_h_file;
    char *boot_c_file;
    char *boot_h_path;
    char *boot_c_path;
    char *boot_func;
};

// Modify a string in place, swapping out "::" for the path separator
// character.
static void
S_double_colons_to_path_sep(char *text);

const static CFCMeta CFCPERL_META = {
    "Clownfish::CFC::Binding::Perl",
    sizeof(CFCPerl),
    (CFCBase_destroy_t)CFCPerl_destroy
};

CFCPerl*
CFCPerl_new(CFCParcel *parcel, CFCHierarchy *hierarchy, const char *lib_dir,
            const char *boot_class, const char *header, const char *footer) {
    CFCPerl *self = (CFCPerl*)CFCBase_allocate(&CFCPERL_META);
    return CFCPerl_init(self, parcel, hierarchy, lib_dir, boot_class, header,
                        footer);
}

CFCPerl*
CFCPerl_init(CFCPerl *self, CFCParcel *parcel, CFCHierarchy *hierarchy,
             const char *lib_dir, const char *boot_class, const char *header,
             const char *footer) {
    CFCUTIL_NULL_CHECK(parcel);
    CFCUTIL_NULL_CHECK(hierarchy);
    CFCUTIL_NULL_CHECK(lib_dir);
    CFCUTIL_NULL_CHECK(boot_class);
    CFCUTIL_NULL_CHECK(header);
    CFCUTIL_NULL_CHECK(footer);
    self->parcel     = (CFCParcel*)CFCBase_incref((CFCBase*)parcel);
    self->hierarchy  = (CFCHierarchy*)CFCBase_incref((CFCBase*)hierarchy);
    self->lib_dir    = CFCUtil_strdup(lib_dir);
    self->boot_class = CFCUtil_strdup(boot_class);
    self->header     = CFCUtil_strdup(header);
    self->footer     = CFCUtil_strdup(footer);

    // Derive path to generated .xs file.
    self->xs_path = CFCUtil_cat(CFCUtil_strdup(""), lib_dir, CFCUTIL_PATH_SEP,
                                boot_class, ".xs", NULL);
    S_double_colons_to_path_sep(self->xs_path);

    // Derive path to generated .pm file.
    self->pm_path = CFCUtil_cat(CFCUtil_strdup(""), lib_dir, CFCUTIL_PATH_SEP,
                                boot_class, CFCUTIL_PATH_SEP,
                                "Autobinding.pm", NULL);
    S_double_colons_to_path_sep(self->pm_path);

    // Derive the name of the files containing bootstrapping code.
    const char *prefix   = CFCParcel_get_prefix(parcel);
    const char *dest_dir = CFCHierarchy_get_dest(hierarchy);
    self->boot_h_file = CFCUtil_cat(CFCUtil_strdup(""), prefix, "boot.h",
                                    NULL);
    self->boot_c_file = CFCUtil_cat(CFCUtil_strdup(""), prefix, "boot.c",
                                    NULL);
    self->boot_h_path = CFCUtil_cat(CFCUtil_strdup(""), dest_dir,
                                    CFCUTIL_PATH_SEP, self->boot_h_file,
                                    NULL);
    self->boot_c_path = CFCUtil_cat(CFCUtil_strdup(""), dest_dir,
                                    CFCUTIL_PATH_SEP, self->boot_c_file,
                                    NULL);

    // Derive the name of the bootstrap function.
    self->boot_func
        = CFCUtil_cat(CFCUtil_strdup(""), CFCParcel_get_prefix(parcel),
                      boot_class, "_bootstrap", NULL);
    for (int i = 0; self->boot_func[i] != 0; i++) {
        if (!isalnum(self->boot_func[i])) {
            self->boot_func[i] = '_';
        }
    }

    return self;
}

void
CFCPerl_destroy(CFCPerl *self) {
    CFCBase_decref((CFCBase*)self->parcel);
    CFCBase_decref((CFCBase*)self->hierarchy);
    FREEMEM(self->lib_dir);
    FREEMEM(self->boot_class);
    FREEMEM(self->header);
    FREEMEM(self->footer);
    FREEMEM(self->xs_path);
    FREEMEM(self->pm_path);
    FREEMEM(self->boot_h_file);
    FREEMEM(self->boot_c_file);
    FREEMEM(self->boot_h_path);
    FREEMEM(self->boot_c_path);
    FREEMEM(self->boot_func);
    CFCBase_destroy((CFCBase*)self);
}

static void
S_double_colons_to_path_sep(char *text) {
    size_t pos = 0;
    for (char *ptr = text; *ptr != '\0'; ptr++) {
        if (strncmp(ptr, "::", 2) == 0) {
            text[pos++] = CFCUTIL_PATH_SEP_CHAR;
            ptr++;
        }
        else {
            text[pos++] = *ptr;
        }
    }
    text[pos] = '\0';
}

char**
CFCPerl_write_pod(CFCPerl *self) {
    CFCPerlClass **registry  = CFCPerlClass_registry();
    size_t num_registered = 0;
    while (registry[num_registered] != NULL) { num_registered++; }
    char     **pod_paths = (char**)CALLOCATE(num_registered + 1, sizeof(char*));
    char     **pods      = (char**)CALLOCATE(num_registered + 1, sizeof(char*));
    CFCClass **ordered   = CFCHierarchy_ordered_classes(self->hierarchy);
    size_t     count     = 0;

    // Generate POD, but don't write.  That way, if there's an error while
    // generating pod, we leak memory but don't clutter up the file system.
    for (size_t i = 0; i < num_registered; i++) {
        const char *class_name = CFCPerlClass_get_class_name(registry[i]);
        char *pod = CFCPerlClass_create_pod(registry[i]);
        if (!pod) { continue; }
        char *pod_path
            = CFCUtil_cat(CFCUtil_strdup(""), self->lib_dir, CFCUTIL_PATH_SEP,
                          class_name, ".pod", NULL);
        S_double_colons_to_path_sep(pod_path);

        pods[count] = pod;
        pod_paths[count] = pod_path;
        count++;
    }

    // Write out any POD files that have changed.
    size_t num_written = 0;
    for (size_t i = 0; i < count; i++) {
        char *pod      = pods[i];
        char *pod_path = pod_paths[i];
        if (CFCUtil_write_if_changed(pod_path, pod, strlen(pod))) {
            pod_paths[num_written] = pod_path;
            num_written++;
        }
        else {
            FREEMEM(pod_path);
        }
        FREEMEM(pod);
    }
    pod_paths[num_written] = NULL;

    return pod_paths;
}

char*
CFCPerl_pm_file_contents(CFCPerl *self, const char *params_hash_defs) {
    const char pattern[] = 
    "# DO NOT EDIT!!!! This is an auto-generated file.\n"
    "\n"
    "# Licensed to the Apache Software Foundation (ASF) under one or more\n"
    "# contributor license agreements.  See the NOTICE file distributed with\n"
    "# this work for additional information regarding copyright ownership.\n"
    "# The ASF licenses this file to You under the Apache License, Version 2.0\n"
    "# (the \"License\"); you may not use this file except in compliance with\n"
    "# the License.  You may obtain a copy of the License at\n"
    "#\n"
    "#     http://www.apache.org/licenses/LICENSE-2.0\n"
    "#\n"
    "# Unless required by applicable law or agreed to in writing, software\n"
    "# distributed under the License is distributed on an \"AS IS\" BASIS,\n"
    "# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
    "# See the License for the specific language governing permissions and\n"
    "# limitations under the License.\n"
    "\n"
    "use strict;\n"
    "use warnings;\n"
    "\n"
    "package Lucy::Autobinding;\n"
    "\n"
    "init_autobindings();\n"
    "\n"
    "%s\n"
    "\n"
    "1;\n"
    "\n";
    size_t size = sizeof(pattern) + strlen(params_hash_defs) + 20;
    char *contents = (char*)MALLOCATE(size);
    sprintf(contents, pattern, params_hash_defs);
    return contents;
}


char*
CFCPerl_xs_file_contents(CFCPerl *self, const char *generated_xs,
                         const char *xs_init, const char *hand_rolled_xs) {
    const char pattern[] = 
    "/* DO NOT EDIT!!!! This is an auto-generated file. */\n"
    "\n"
    "/* Licensed to the Apache Software Foundation (ASF) under one or more\n"
    " * contributor license agreements.  See the NOTICE file distributed with\n"
    " * this work for additional information regarding copyright ownership.\n"
    " * The ASF licenses this file to You under the Apache License, Version 2.0\n"
    " * (the \"License\"); you may not use this file except in compliance with\n"
    " * the License.  You may obtain a copy of the License at\n"
    " *\n"
    " *     http://www.apache.org/licenses/LICENSE-2.0\n"
    " *\n"
    " * Unless required by applicable law or agreed to in writing, software\n"
    " * distributed under the License is distributed on an \"AS IS\" BASIS,\n"
    " * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
    " * See the License for the specific language governing permissions and\n"
    " * limitations under the License.\n"
    " */\n"
    "\n"
    "#include \"XSBind.h\"\n"
    "#include \"parcel.h\"\n"
    "#include \"%s\"\n"
    "\n"
    "#include \"Lucy/Object/Host.h\"\n"
    "#include \"Lucy/Util/Memory.h\"\n"
    "#include \"Lucy/Util/StringHelper.h\"\n"
    "\n"
    "%s\n"
    "\n"
    "MODULE = Lucy   PACKAGE = Lucy::Autobinding\n"
    "\n"
    "void\n"
    "init_autobindings()\n"
    "PPCODE:\n"
    "{\n"
    "    char* file = __FILE__;\n"
    "    CHY_UNUSED_VAR(cv);\n"
    "    CHY_UNUSED_VAR(items); %s\n"
    "}\n"
    "\n"
    "%s\n"
    "\n";

    size_t size = sizeof(pattern)
                  + strlen(self->boot_h_file)
                  + strlen(generated_xs)
                  + strlen(xs_init)
                  + strlen(hand_rolled_xs)
                  + 30;
    char *contents = (char*)MALLOCATE(size);
    sprintf(contents, pattern, self->boot_h_file, generated_xs, xs_init,
            hand_rolled_xs);

    return contents;
}

CFCParcel*
CFCPerl_get_parcel(CFCPerl *self) {
    return self->parcel;
}

CFCHierarchy*
CFCPerl_get_hierarchy(CFCPerl *self) {
    return self->hierarchy;
}

const char*
CFCPerl_get_lib_dir(CFCPerl *self) {
    return self->lib_dir;
}

const char*
CFCPerl_get_boot_class(CFCPerl *self) {
    return self->boot_class;
}

const char*
CFCPerl_get_header(CFCPerl *self) {
    return self->header;
}

const char*
CFCPerl_get_footer(CFCPerl *self) {
    return self->footer;
}

const char*
CFCPerl_get_xs_path(CFCPerl *self) {
    return self->xs_path;
}

const char*
CFCPerl_get_pm_path(CFCPerl *self) {
    return self->pm_path;
}

const char*
CFCPerl_get_boot_h_file(CFCPerl *self) {
    return self->boot_h_file;
}

const char*
CFCPerl_get_boot_c_file(CFCPerl *self) {
    return self->boot_c_file;
}

const char*
CFCPerl_get_boot_h_path(CFCPerl *self) {
    return self->boot_h_path;
}

const char*
CFCPerl_get_boot_c_path(CFCPerl *self) {
    return self->boot_c_path;
}

const char*
CFCPerl_get_boot_func(CFCPerl *self) {
    return self->boot_func;
}

