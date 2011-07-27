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
#include "CFCPerlTypeMap.h"
#include "CFCUtil.h"
#include "CFCHierarchy.h"
#include "CFCClass.h"

#ifndef true
    #define true 1
    #define false 0
#endif

static const char typemap_start[] =
    "# Auto-generated file.\n"
    "\n"
    "TYPEMAP\n"
    "chy_bool_t\tCHY_BOOL\n"
    "int8_t\tCHY_SIGNED_INT\n"
    "int16_t\tCHY_SIGNED_INT\n"
    "int32_t\tCHY_SIGNED_INT\n"
    "int64_t\tCHY_BIG_SIGNED_INT\n"
    "uint8_t\tCHY_UNSIGNED_INT\n"
    "uint16_t\tCHY_UNSIGNED_INT\n"
    "uint32_t\tCHY_UNSIGNED_INT\n"
    "uint64_t\tCHY_BIG_UNSIGNED_INT\n"
    "\n"
    "const lucy_CharBuf*\tCONST_CHARBUF\n";


static const char typemap_input[] =
    "INPUT\n"
    "\n"
    "CHY_BOOL\n"
    "    $var = ($type)SvTRUE($arg);\n"
    "\n"
    "CHY_SIGNED_INT \n"
    "    $var = ($type)SvIV($arg);\n"
    "\n"
    "CHY_UNSIGNED_INT\n"
    "    $var = ($type)SvUV($arg);\n"
    "\n"
    "CHY_BIG_SIGNED_INT \n"
    "    $var = (sizeof(IV) == 8) ? ($type)SvIV($arg) : ($type)SvNV($arg);\n"
    "\n"
    "CHY_BIG_UNSIGNED_INT \n"
    "    $var = (sizeof(UV) == 8) ? ($type)SvUV($arg) : ($type)SvNV($arg);\n"
    "\n"
    "CONST_CHARBUF\n"
    "    $var = (const cfish_CharBuf*)CFISH_ZCB_WRAP_STR(SvPVutf8_nolen($arg), SvCUR($arg));\n"
    "\n";

static const char typemap_output[] =
    "OUTPUT\n"
    "\n"
    "CHY_BOOL\n"
    "    sv_setiv($arg, (IV)$var);\n"
    "\n"
    "CHY_SIGNED_INT\n"
    "    sv_setiv($arg, (IV)$var);\n"
    "\n"
    "CHY_UNSIGNED_INT\n"
    "    sv_setuv($arg, (UV)$var);\n"
    "\n"
    "CHY_BIG_SIGNED_INT\n"
    "    if (sizeof(IV) == 8) { sv_setiv($arg, (IV)$var); }\n"
    "    else                 { sv_setnv($arg, (NV)$var); }\n"
    "\n"
    "CHY_BIG_UNSIGNED_INT\n"
    "    if (sizeof(UV) == 8) { sv_setuv($arg, (UV)$var); }\n"
    "    else                 { sv_setnv($arg, (NV)$var); }\n"
    "\n";

void
CFCPerlTypeMap_write_xs_typemap(CFCHierarchy *hierarchy) {
    CFCClass **classes = CFCHierarchy_ordered_classes(hierarchy);
    char *start  = CFCUtil_strdup("");
    char *input  = CFCUtil_strdup("");
    char *output = CFCUtil_strdup("");
    for (int i = 0; classes[i] != NULL; i++) {
        CFCClass *klass = classes[i];
        const char *full_struct_sym = CFCClass_full_struct_sym(klass);
        const char *vtable_var      = CFCClass_full_vtable_var(klass);

        start = CFCUtil_cat(start, full_struct_sym, "*\t", vtable_var, "_\n",
                            NULL);
        input = CFCUtil_cat(input, vtable_var, "_\n"
                            "    $var = (", full_struct_sym,
                            "*)XSBind_sv_to_cfish_obj($arg, ", vtable_var,
                            ", NULL);\n\n", NULL);

        output = CFCUtil_cat(output, vtable_var, "_\n"
            "    $arg = (SV*)Cfish_Obj_To_Host((cfish_Obj*)$var);\n"
            "    LUCY_DECREF($var);\n"
            "\n", NULL);
    }

    char *content = CFCUtil_strdup("");
    content = CFCUtil_cat(content, typemap_start, start, "\n\n",
                          typemap_input, input, "\n\n", 
                          typemap_output, output, "\n\n", NULL);
    CFCUtil_write_if_changed("typemap", content, strlen(content));

    FREEMEM(content);
    FREEMEM(output);
    FREEMEM(input);
    FREEMEM(start);
}

