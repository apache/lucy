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
#include "CFCBindAliases.h"
#include "CFCUtil.h"

struct alias {
    const char *from;
    const char *to;
};

struct alias aliases[] = {
    {NULL, NULL}
};

char*
CFCBindAliases_c_aliases(void) {
    size_t size = 200;
    for (int i = 0; aliases[i].from != NULL; i++) {
        size += strlen(aliases[i].from);
        size += strlen(aliases[i].to);
        size += sizeof("#define %s %s\n");
    }
    char *content = (char*)MALLOCATE(size);
    content[0] = '\0';

    strcat(content, "#ifndef CFISH_C_ALIASES\n#define CFISH_C_ALIASES\n\n");
    for (int i = 0; aliases[i].from != NULL; i++) {
        strcat(content, "#define ");
        strcat(content, aliases[i].from);
        strcat(content, " ");
        strcat(content, aliases[i].to);
        strcat(content, "\n");
    }
    strcat(content, "\n#endif /* CFISH_C_ALIASES */\n\n");

    return content;
}

