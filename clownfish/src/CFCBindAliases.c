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
    {"cfish_ref_t", "lucy_ref_t"},
    {"cfish_method_t", "lucy_method_t"},
    {"cfish_method", "lucy_method"},
    {"cfish_super_method", "lucy_super_method"},

    {"cfish_Obj", "lucy_Obj"},
    {"CFISH_OBJ", "LUCY_OBJ"},
    {"Cfish_Obj_Dump", "Lucy_Obj_Dump"},
    {"Cfish_Obj_Get_Class_Name", "Lucy_Obj_Get_Class_Name"},
    {"Cfish_Obj_Is_A", "Lucy_Obj_Is_A"},
    {"Cfish_Obj_Load", "Lucy_Obj_Load"},
    {"Cfish_Obj_To_F64", "Lucy_Obj_To_F64"},
    {"Cfish_Obj_To_I64", "Lucy_Obj_To_I64"},
    {"Cfish_Obj_To_Bool", "Lucy_Obj_To_Bool"},
    {"Cfish_Obj_To_Host", "Lucy_Obj_To_Host"},
    {"Cfish_Obj_Dec_RefCount", "Lucy_Obj_Dec_RefCount"},
    {"Cfish_Obj_Inc_RefCount", "Lucy_Obj_Inc_RefCount"},

    {"cfish_ByteBuf", "lucy_ByteBuf"},
    {"CFISH_BYTEBUF", "LUCY_BYTEBUF"},
    {"Cfish_BB_Get_Size", "Lucy_BB_Get_Size"},
    {"Cfish_BB_Get_Buf", "Lucy_BB_Get_Buf"},

    {"cfish_CharBuf", "lucy_CharBuf"},
    {"CFISH_CHARBUF", "LUCY_CHARBUF"},
    {"cfish_CB_newf", "lucy_CB_newf"},
    {"cfish_CB_new_from_trusted_utf8", "lucy_CB_new_from_trusted_utf8"},
    {"Cfish_CB_Clone", "Lucy_CB_Clone"},
    {"cfish_ZombieCharBuf", "lucy_ZombieCharBuf"},
    {"CFISH_ZOMBIECHARBUF", "LUCY_ZOMBIECHARBUF"},
    {"CFISH_VIEWCHARBUF", "LUCY_VIEWCHARBUF"},
    {"cfish_ZCB_size", "lucy_ZCB_size"},
    {"cfish_ZCB_wrap_str", "lucy_ZCB_wrap_str"},
    {"Cfish_ZCB_Assign_Str", "Lucy_ZCB_Assign_Str"},
    {"Cfish_ZCB_Assign_Trusted_Str", "Lucy_ZCB_Assign_Trusted_Str"},
    {"Cfish_CB_Get_Ptr8", "Lucy_CB_Get_Ptr8"},
    {"Cfish_CB_Get_Size", "Lucy_CB_Get_Size"},

    {"CFISH_FLOATNUM", "LUCY_FLOATNUM"},
    {"CFISH_INTNUM", "LUCY_INTNUM"},
    {"CFISH_INTEGER32", "LUCY_INTEGER32"},
    {"CFISH_INTEGER64", "LUCY_INTEGER64"},
    {"CFISH_FLOAT32", "LUCY_FLOAT32"},
    {"CFISH_FLOAT64", "LUCY_FLOAT64"},
    {"cfish_Bool_singleton", "lucy_Bool_singleton"},

    {"cfish_Err", "lucy_Err"},
    {"CFISH_ERR", "LUCY_ERR"},
    {"cfish_Err_new", "lucy_Err_new"},
    {"cfish_Err_trap", "lucy_Err_trap"},
    {"cfish_Err_set_error", "lucy_Err_set_error"},
    {"cfish_Err_get_error", "lucy_Err_get_error"},

    {"cfish_Hash", "lucy_Hash"},
    {"CFISH_HASH", "LUCY_HASH"},
    {"cfish_Hash_new", "lucy_Hash_new"},
    {"Cfish_Hash_Iterate", "Lucy_Hash_Iterate"},
    {"Cfish_Hash_Next", "Lucy_Hash_Next"},
    {"Cfish_Hash_Fetch_Str", "Lucy_Hash_Fetch_Str"},
    {"Cfish_Hash_Store_Str", "Lucy_Hash_Store_Str"},
    {"Cfish_Hash_Store", "Lucy_Hash_Store"},

    {"cfish_Method", "lucy_Method"},
    {"CFISH_METHOD", "LUCY_METHOD"},

    {"cfish_VArray", "lucy_VArray"},
    {"CFISH_VARRAY", "LUCY_VARRAY"},
    {"cfish_VA_new", "lucy_VA_new"},
    {"Cfish_VA_Fetch", "Lucy_VA_Fetch"},
    {"Cfish_VA_Get_Size", "Lucy_VA_Get_Size"},
    {"Cfish_VA_Resize", "Lucy_VA_Resize"},
    {"Cfish_VA_Store", "Lucy_VA_Store"},
    {"Cfish_VA_Push", "Lucy_VA_Push"},

    {"cfish_VTable", "lucy_VTable"},
    {"CFISH_VTABLE", "LUCY_VTABLE"},
    {"cfish_VTable_allocate", "lucy_VTable_allocate"},
    {"cfish_VTable_init", "lucy_VTable_init"},
    {"cfish_VTable_add_to_registry", "lucy_VTable_add_to_registry"},
    {"cfish_VTable_add_alias_to_registry", "lucy_VTable_add_alias_to_registry"},
    {"cfish_VTable_offset_of_parent", "lucy_VTable_offset_of_parent"},
    {"cfish_VTable_override", "lucy_VTable_override"},
    {"cfish_VTable_singleton", "lucy_VTable_singleton"},
    {"Cfish_VTable_Get_Name", "Lucy_VTable_Get_Name"},
    {"Cfish_VTable_Make_Obj", "Lucy_VTable_Make_Obj"},

    {"cfish_Host_callback", "lucy_Host_callback"},
    {"cfish_Host_callback_f64", "lucy_Host_callback_f64"},
    {"cfish_Host_callback_host", "lucy_Host_callback_host"},
    {"cfish_Host_callback_i64", "lucy_Host_callback_i64"},
    {"cfish_Host_callback_obj", "lucy_Host_callback_obj"},
    {"cfish_Host_callback_str", "lucy_Host_callback_str"},

    {"CFISH_USE_SHORT_NAMES", "LUCY_USE_SHORT_NAMES"},

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

