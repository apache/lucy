#define CHAZ_USE_SHORT_NAMES

#include "Charmonizer/Core/ModHandler.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Probe/FuncMacro.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* code for verifying ISO func macro */
static char iso_func_code[] = METAQUOTE
    #include "_charm.h" 
    int main() {
        Charm_Setup;
        printf("%s", __func__);
        return 0;
    }
METAQUOTE;

/* code for verifying GNU func macro */
static char gnuc_func_code[] = METAQUOTE
    #include "_charm.h" 
    int main() {
        Charm_Setup;
        printf("%s", __FUNCTION__);
        return 0;
    }
METAQUOTE;

/* code for verifying inline keyword */
static char inline_code[] = METAQUOTE
    #include "_charm.h" 
    static %s int foo() { return 1; }
    int main() {
        Charm_Setup;
        printf("%%d", foo());
        return 0;
    }
METAQUOTE;

static char*
S_try_inline(const char *keyword, size_t *output_len) {
    char code[ sizeof(inline_code) + 30 ];
    sprintf(code, inline_code, keyword);
    return ModHand_capture_output(code, strlen(code), output_len);
}

static const char* inline_options[] = {
    "__inline",
    "__inline__",
    "inline"
};
static int num_inline_options = sizeof(inline_options) / sizeof(void*);

void
FuncMacro_run(void) 
{
    int i;
    char *output;
    size_t output_len;
    chaz_bool_t has_funcmac      = false;
    chaz_bool_t has_iso_funcmac  = false;
    chaz_bool_t has_gnuc_funcmac = false;
    chaz_bool_t has_inline       = false;

    START_RUN("FuncMacro");
    
    /* check for ISO func macro */
    output = ModHand_capture_output(iso_func_code, strlen(iso_func_code), 
        &output_len);
    if (output != NULL && strncmp(output, "main", 4) == 0) {
        has_funcmac     = true;
        has_iso_funcmac = true;
    }
    free(output);

    /* check for GNUC func macro */
    output = ModHand_capture_output(gnuc_func_code, strlen(gnuc_func_code), 
        &output_len);
    if (output != NULL && strncmp(output, "main", 4) == 0) {
        has_funcmac      = true;
        has_gnuc_funcmac = true;
    }
    free(output);

    /* write out common defines */
    if (has_funcmac) {
        const char *macro_text = has_iso_funcmac 
            ? "__func__"
            : "__FUNCTION__";
        ModHand_append_conf(
            "#define CHY_HAS_FUNC_MACRO\n"
            "#define CHY_FUNC_MACRO %s\n",
            macro_text
        );
    }

    /* write out specific defines */
    if (has_iso_funcmac) {
       ModHand_append_conf("#define CHY_HAS_ISO_FUNC_MACRO\n");
    }
    if (has_gnuc_funcmac) {
        ModHand_append_conf("#define CHY_HAS_GNUC_FUNC_MACRO\n");
    }

    /* Check for inline keyword. */
    
    for (i = 0; i < num_inline_options; i++) {
        const char *inline_option = inline_options[i];
        output = S_try_inline(inline_option, &output_len);
        if (output != NULL) {
            has_inline = true;
            ModHand_append_conf("#define CHY_INLINE %s\n", inline_option);
            free(output);
            break;
        }
    }
    if (!has_inline) {
        ModHand_append_conf("#define CHY_INLINE\n");
    }

    /* shorten */
    START_SHORT_NAMES;
    if (has_iso_funcmac) 
        ModHand_shorten_macro("HAS_ISO_FUNC_MACRO");
    if (has_gnuc_funcmac)
        ModHand_shorten_macro("HAS_GNUC_FUNC_MACRO");
    if (has_iso_funcmac || has_gnuc_funcmac) {
        ModHand_shorten_macro("HAS_FUNC_MACRO");
        ModHand_shorten_macro("FUNC_MACRO");
    }
    ModHand_shorten_macro("INLINE");
    END_SHORT_NAMES;

    END_RUN;
}


/**
 * Copyright 2006 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

