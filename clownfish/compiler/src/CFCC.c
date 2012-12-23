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

#include <stdio.h>
#include <string.h>

#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCC.h"
#include "CFCCClass.h"
#include "CFCClass.h"
#include "CFCHierarchy.h"
#include "CFCUtil.h"

struct CFCC {
    CFCBase base;
    CFCHierarchy *hierarchy;
    char         *header;
    char         *footer;
};

const static CFCMeta CFCC_META = {
    "Clownfish::CFC::Binding::C",
    sizeof(CFCC),
    (CFCBase_destroy_t)CFCC_destroy
};

CFCC*
CFCC_new(CFCHierarchy *hierarchy, const char *header, const char *footer) {
    CFCC *self = (CFCC*)CFCBase_allocate(&CFCC_META);
    return CFCC_init(self, hierarchy, header, footer);
}

CFCC*
CFCC_init(CFCC *self, CFCHierarchy *hierarchy, const char *header,
          const char *footer) {
    CFCUTIL_NULL_CHECK(hierarchy);
    CFCUTIL_NULL_CHECK(header);
    CFCUTIL_NULL_CHECK(footer);
    self->hierarchy = (CFCHierarchy*)CFCBase_incref((CFCBase*)hierarchy);
    self->header    = CFCUtil_strdup(header);
    self->footer    = CFCUtil_strdup(footer);
    return self;
}

void
CFCC_destroy(CFCC *self) {
    CFCBase_decref((CFCBase*)self->hierarchy);
    FREEMEM(self->header);
    FREEMEM(self->footer);
    CFCBase_destroy((CFCBase*)self);
}

/* Write "callbacks.h" with NULL callbacks.
 */
void
CFCC_write_callbacks(CFCC *self) {
    CFCHierarchy  *hierarchy   = self->hierarchy;
    CFCClass     **ordered     = CFCHierarchy_ordered_classes(hierarchy);
    char          *all_cb_decs = CFCUtil_strdup("");

    for (int i = 0; ordered[i] != NULL; i++) {
        CFCClass *klass = ordered[i];

        if (!CFCClass_included(klass)) {
            char *cb_decs = CFCCClass_callback_decs(klass);
            all_cb_decs = CFCUtil_cat(all_cb_decs, cb_decs, NULL);
            FREEMEM(cb_decs);
        }
    }

    FREEMEM(ordered);

    const char pattern[] =
        "%s\n"
        "#ifndef CFCCALLBACKS_H\n"
        "#define CFCCALLBACKS_H 1\n"
        "\n"
        "#include <stddef.h>\n"
        "\n"
        "%s"
        "\n"
        "#endif /* CFCCALLBACKS_H */\n"
        "\n"
        "%s\n"
        "\n";
    char *file_content = CFCUtil_sprintf(pattern, self->header, all_cb_decs,
                                         self->footer);

    // Unlink then write file.
    const char *inc_dest = CFCHierarchy_get_include_dest(hierarchy);
    char *filepath = CFCUtil_sprintf("%s" CHY_DIR_SEP "callbacks.h", inc_dest);
    remove(filepath);
    CFCUtil_write_file(filepath, file_content, strlen(file_content));
    FREEMEM(filepath);

    FREEMEM(all_cb_decs);
    FREEMEM(file_content);
}


