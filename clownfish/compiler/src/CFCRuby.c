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

#include "charmony.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCClass.h"
#include "CFCHierarchy.h"
#include "CFCParcel.h"
#include "CFCRuby.h"
#include "CFCUtil.h"

struct CFCRuby {
    CFCBase base;
    CFCParcel *parcel;
    CFCHierarchy *hierarchy;
    char *lib_dir;
    char *boot_class;
    char *header;
    char *footer;
    char *boot_h_file;
    char *boot_c_file;
    char *boot_h_path;
    char *boot_c_path;
    char *boot_func;
};

// Modify a string in place, swapping out "::" for the supplied character.
static void
S_replace_double_colons(char *text, char replacement);

static const CFCMeta CFCRUBY_META = {
    "Clownfish::CFC::Binding::Ruby",
    sizeof(CFCRuby),
    (CFCBase_destroy_t)CFCRuby_destroy
};

CFCRuby*
CFCRuby_new(CFCParcel *parcel, CFCHierarchy *hierarchy, const char *lib_dir,
            const char *boot_class, const char *header, const char *footer) {
    CFCRuby *self = (CFCRuby*)CFCBase_allocate(&CFCRUBY_META);
    return CFCRuby_init(self, parcel, hierarchy, lib_dir, boot_class, header,
                        footer);
}

CFCRuby*
CFCRuby_init(CFCRuby *self, CFCParcel *parcel, CFCHierarchy *hierarchy,
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

    const char *prefix   = CFCParcel_get_prefix(parcel);
    const char *inc_dest = CFCHierarchy_get_include_dest(hierarchy);
    const char *src_dest = CFCHierarchy_get_source_dest(hierarchy);
    self->boot_h_file = CFCUtil_sprintf("%sboot.h", prefix);
    self->boot_c_file = CFCUtil_sprintf("%sboot.c", prefix);
    self->boot_h_path = CFCUtil_sprintf("%s" CHY_DIR_SEP "%s", inc_dest,
                                        self->boot_h_file);
    self->boot_c_path = CFCUtil_sprintf("%s" CHY_DIR_SEP "%s", src_dest,
                                        self->boot_c_file);
    self->boot_func = CFCUtil_sprintf("%s%s_bootstrap", prefix, boot_class);

    for (int i = 0; self->boot_func[i] != 0; i++) {
        if (!isalnum(self->boot_func[i])) {
            self->boot_func[i] = '_';
        }
    }

    return self;
}

void
CFCRuby_destroy(CFCRuby *self) {
    CFCBase_decref((CFCBase*)self->parcel);
    CFCBase_decref((CFCBase*)self->hierarchy);
    FREEMEM(self->lib_dir);
    FREEMEM(self->boot_class);
    FREEMEM(self->header);
    FREEMEM(self->footer);
    FREEMEM(self->boot_h_file);
    FREEMEM(self->boot_c_file);
    FREEMEM(self->boot_h_path);
    FREEMEM(self->boot_c_path);
    FREEMEM(self->boot_func);
    CFCBase_destroy((CFCBase*)self);
}

static void
S_replace_double_colons(char *text, char replacement) {
    size_t pos = 0;
    for (char *ptr = text; *ptr != '\0'; ptr++) {
        if (strncmp(ptr, "::", 2) == 0) {
            text[pos++] = replacement;
            ptr++;
        }
        else {
            text[pos++] = *ptr;
        }
    }
    text[pos] = '\0';
}

static void
S_write_boot_h(CFCRuby *self) {
    char *guard = CFCUtil_cat(CFCUtil_strdup(""), self->boot_class,
                              "_BOOT", NULL);
    S_replace_double_colons(guard, '_');
    for (char *ptr = guard; *ptr != '\0'; ptr++) {
        if (isalpha(*ptr)) {
            *ptr = toupper(*ptr);
        }
    }

    const char pattern[] = 
        "%s\n"
        "\n"
        "#ifndef %s\n"
        "#define %s 1\n"
        "\n"
        "void\n"
        "%s();\n"
        "\n"
        "#endif /* %s */\n"
        "\n"
        "%s\n";

    size_t size = sizeof(pattern)
                  + strlen(self->header)
                  + strlen(guard)
                  + strlen(guard)
                  + strlen(self->boot_func)
                  + strlen(guard)
                  + strlen(self->footer)
                  + 20;
    char *content = (char*)MALLOCATE(size);
    sprintf(content, pattern, self->header, guard, guard, self->boot_func,
            guard, self->footer);
    CFCUtil_write_file(self->boot_h_path, content, strlen(content));

    FREEMEM(content);
    FREEMEM(guard);
}

static void
S_write_boot_c(CFCRuby *self) {
    CFCClass **ordered   = CFCHierarchy_ordered_classes(self->hierarchy);
    char *pound_includes = CFCUtil_strdup("");
    const char *prefix   = CFCParcel_get_prefix(self->parcel);

    for (size_t i = 0; ordered[i] != NULL; i++) {
        CFCClass *klass = ordered[i];
        if (CFCClass_included(klass)) { continue; }

        const char *include_h  = CFCClass_include_h(klass);
        pound_includes = CFCUtil_cat(pound_includes, "#include \"",
                                     include_h, "\"\n", NULL);

        if (CFCClass_inert(klass)) { continue; }

        CFCClass *parent = CFCClass_get_parent(klass);
        if (parent) {
            /* Need to implement */
        }
    }

    const char pattern[] =
        "%s\n"
        "\n"
        "#include \"charmony.h\"\n"
        "#include \"%s\"\n"
        "#include \"parcel.h\"\n"
        "#include \"Clownfish/CharBuf.h\"\n"
        "#include \"Clownfish/VTable.h\"\n"
        "%s\n"
        "\n"
        "void\n"
        "%s() {\n"
        "    %sbootstrap_parcel();\n"
        "\n"
        "    cfish_ZombieCharBuf *alias = CFISH_ZCB_WRAP_STR(\"\", 0);\n"
        "}\n"
        "\n"
        "%s\n"
        "\n";

    size_t size = sizeof(pattern)
                  + strlen(self->header)
                  + strlen(self->boot_h_file)
                  + strlen(pound_includes)
                  + strlen(self->boot_func)
                  + strlen(prefix)
                  + strlen(self->footer)
                  + 100;
    char *content = (char*)MALLOCATE(size);
    sprintf(content, pattern, self->header, self->boot_h_file, pound_includes,
            self->boot_func, prefix, self->footer);
    CFCUtil_write_file(self->boot_c_path, content, strlen(content));

    FREEMEM(content);
    FREEMEM(pound_includes);
    FREEMEM(ordered);
}

void
CFCRuby_write_boot(CFCRuby *self) {
    S_write_boot_h(self);
    S_write_boot_c(self);
}

void
CFCRuby_write_hostdefs(CFCRuby *self) {
    const char pattern[] =
        "%s\n"
        "\n"
        "#ifndef H_CFISH_HOSTDEFS\n"
        "#define H_CFISH_HOSTDEFS 1\n"
        "\n"
        "#define CFISH_OBJ_HEAD\n"
        "\n"
        "#endif /* H_CFISH_HOSTDEFS */\n"
        "\n"
        "%s\n";
    char *content
        = CFCUtil_sprintf(pattern, self->header, self->footer);

    // Unlink then write file.
    const char *inc_dest = CFCHierarchy_get_include_dest(self->hierarchy);
    char *filepath = CFCUtil_sprintf("%s" CHY_DIR_SEP "hostdefs.h", inc_dest);
    remove(filepath);
    CFCUtil_write_file(filepath, content, strlen(content));
    FREEMEM(filepath);

    FREEMEM(content);
}

