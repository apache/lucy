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
#include "CFCPerlTypeMap.h"
#include "CFCUtil.h"
#include "CFCHierarchy.h"
#include "CFCClass.h"
#include "CFCType.h"

#ifndef true
    #define true 1
    #define false 0
#endif

// Convert from a Perl scalar to a primitive type.
struct char_map {
    char *key;
    char *value;
};


char*
CFCPerlTypeMap_from_perl(CFCType *type, const char *xs_var) {
    char *result = NULL;

    if (CFCType_is_object(type)) {
        const char *struct_sym = CFCType_get_specifier(type);
        const char *vtable_var = CFCType_get_vtable_var(type);
        if (strcmp(struct_sym, "lucy_CharBuf") == 0
            || strcmp(struct_sym, "cfish_CharBuf") == 0
            || strcmp(struct_sym, "lucy_Obj") == 0
            || strcmp(struct_sym, "cfish_Obj") == 0
           ) {
            // Share buffers rather than copy between Perl scalars and
            // Clownfish string types.
            result = CFCUtil_cat(CFCUtil_strdup(""), "(", struct_sym,
                                 "*)XSBind_sv_to_cfish_obj(", xs_var, 
                                 ", ", vtable_var, 
                                 ", alloca(cfish_ZCB_size()))", NULL);
        }
        else {
            result = CFCUtil_cat(CFCUtil_strdup(""), "(", struct_sym,
                                 "*)XSBind_sv_to_cfish_obj(", xs_var, 
                                 ", ", vtable_var, ", NULL)", NULL);
        }
    }
    else if (CFCType_is_primitive(type)) {
        const char *specifier = CFCType_get_specifier(type);
        size_t size = 80 + strlen(xs_var) * 2;
        result = (char*)MALLOCATE(size);

        if (strcmp(specifier, "double") == 0) {
            sprintf(result, "SvNV(%s)", xs_var);
        }
        else if (strcmp(specifier, "float") == 0) {
            sprintf(result, "(float)SvNV(%s)", xs_var);
        }
        else if (strcmp(specifier, "int") == 0) {
            sprintf(result, "(int)SvIV(%s)", xs_var);
        }
        else if (strcmp(specifier, "short") == 0) {
            sprintf(result, "(short)SvIV(%s)", xs_var);
        }
        else if (strcmp(specifier, "long") == 0) {
            const char pattern[] =
                "((sizeof(long) <= sizeof(IV)) ? (long)SvIV(%s) "
                ": (long)SvNV(%s))";
            sprintf(result, pattern, xs_var, xs_var);
        }
        else if (strcmp(specifier, "size_t") == 0) {
            sprintf(result, "(size_t)SvIV(%s)", xs_var);
        }
        else if (strcmp(specifier, "uint64_t") == 0) {
            sprintf(result, "(uint64_t)SvNV(%s)", xs_var);
        }
        else if (strcmp(specifier, "uint32_t") == 0) {
            sprintf(result, "(uint32_t)SvUV(%s)", xs_var);
        }
        else if (strcmp(specifier, "uint16_t") == 0) {
            sprintf(result, "(uint16_t)SvUV(%s)", xs_var);
        }
        else if (strcmp(specifier, "uint8_t") == 0) {
            sprintf(result, "(uint8_t)SvUV(%s)", xs_var);
        }
        else if (strcmp(specifier, "int64_t") == 0) {
            sprintf(result, "(int64_t)SvNV(%s)", xs_var);
        }
        else if (strcmp(specifier, "int32_t") == 0) {
            sprintf(result, "(int32_t)SvIV(%s)", xs_var);
        }
        else if (strcmp(specifier, "int16_t") == 0) {
            sprintf(result, "(int16_t)SvIV(%s)", xs_var);
        }
        else if (strcmp(specifier, "int8_t") == 0) {
            sprintf(result, "(int8_t)SvIV(%s)", xs_var);
        }
        else if (strcmp(specifier, "chy_bool_t") == 0) {
            sprintf(result, "SvTRUE(%s) ? 1 : 0", xs_var);
        }
        else {
            FREEMEM(result);
            result = NULL;
        }
    }

    if (!result) {
        CFCUtil_die("Missing typemap for '%s'", CFCType_to_c(type));
    }

    return result;
}

char*
CFCPerlTypeMap_to_perl(CFCType *type, const char *cf_var) {
    const char *type_str = CFCType_to_c(type);
    char *result = NULL;

    if (CFCType_is_object(type)) {
        result = CFCUtil_cat(CFCUtil_strdup(""), "(", cf_var,
                             " == NULL ? newSV(0) : " 
                             "XSBind_cfish_to_perl((cfish_Obj*)", 
                             cf_var, "))", NULL);
    }
    else if (CFCType_is_primitive(type)) {
        // Convert from a primitive type to a Perl scalar.
        const char *specifier = CFCType_get_specifier(type);
        size_t size = 80 + strlen(cf_var) * 2;
        result = (char*)MALLOCATE(size);

        if (strcmp(specifier, "double") == 0) {
            sprintf(result, "newSVnv(%s)", cf_var);
        }
        else if (strcmp(specifier, "float") == 0) {
            sprintf(result, "newSVnv(%s)", cf_var);
        }
        else if (strcmp(specifier, "int") == 0) {
            sprintf(result, "newSViv(%s)", cf_var);
        }
        else if (strcmp(specifier, "short") == 0) {
            sprintf(result, "newSViv(%s)", cf_var);
        }
        else if (strcmp(specifier, "long") == 0) {
            char pattern[] = 
                "((sizeof(long) <= sizeof(IV)) ? "
                "newSViv((IV)%s) : newSVnv((NV)%s))";
            sprintf(result, pattern, cf_var, cf_var);
        }
        else if (strcmp(specifier, "size_t") == 0) {
            sprintf(result, "newSViv(%s)", cf_var);
        }
        else if (strcmp(specifier, "uint64_t") == 0) {
            char pattern[] = "sizeof(UV) == 8 ? newSVuv((UV)%s) : newSVnv((NV)%s)";
            sprintf(result, pattern, cf_var, cf_var);
        }
        else if (strcmp(specifier, "uint32_t") == 0) {
            sprintf(result, "newSVuv(%s)", cf_var);
        }
        else if (strcmp(specifier, "uint16_t") == 0) {
            sprintf(result, "newSVuv(%s)", cf_var);
        }
        else if (strcmp(specifier, "uint8_t") == 0) {
            sprintf(result, "newSVuv(%s)", cf_var);
        }
        else if (strcmp(specifier, "int64_t") == 0) {
            char pattern[] = "sizeof(IV) == 8 ? newSViv((IV)%s) : newSVnv((NV)%s)";
            sprintf(result, pattern, cf_var, cf_var);
        }
        else if (strcmp(specifier, "int32_t") == 0) {
            sprintf(result, "newSViv(%s)", cf_var);
        }
        else if (strcmp(specifier, "int16_t") == 0) {
            sprintf(result, "newSViv(%s)", cf_var);
        }
        else if (strcmp(specifier, "int8_t") == 0) {
            sprintf(result, "newSViv(%s)", cf_var);
        }
        else if (strcmp(specifier, "chy_bool_t") == 0) {
            sprintf(result, "newSViv(%s)", cf_var);
        }
        else {
            FREEMEM(result);
            result = NULL;
        }
    }
    else if (CFCType_is_composite(type)) {
        if (strcmp(type_str, "void*") == 0) {
            // Assume that void* is a reference SV -- either a hashref or an
            // arrayref.
            result = CFCUtil_cat(CFCUtil_strdup(""), "newRV_inc((SV*)", 
                                 cf_var,")", NULL);
        }
    }

    if (!result) {
        CFCUtil_die("Missing typemap for '%s'", CFCType_to_c(type));
    }

    return result;
}

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
            "    CFISH_DECREF($var);\n"
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

