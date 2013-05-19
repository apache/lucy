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
    {"CFISH_VISIBLE", "LUCY_VISIBLE"},

    {"cfish_Obj", "lucy_Obj"},
    {"CFISH_OBJ", "LUCY_OBJ"},
    {"cfish_Obj_dec_refcount", "lucy_Obj_dec_refcount"},
    {"cfish_Obj_get_refcount", "lucy_Obj_get_refcount"},
    {"cfish_Obj_inc_refcount", "lucy_Obj_inc_refcount"},
    {"cfish_Obj_to_host", "lucy_Obj_to_host"},
    {"Cfish_Obj_Dec_RefCount", "Lucy_Obj_Dec_RefCount"},
    {"Cfish_Obj_Deserialize", "Lucy_Obj_Deserialize"},
    {"Cfish_Obj_Destroy", "Lucy_Obj_Destroy"},
    {"Cfish_Obj_Destroy_OFFSET", "Lucy_Obj_Destroy_OFFSET"},
    {"Cfish_Obj_Destroy_t", "Lucy_Obj_Destroy_t"},
    {"Cfish_Obj_Dump", "Lucy_Obj_Dump"},
    {"Cfish_Obj_Get_Class_Name", "Lucy_Obj_Get_Class_Name"},
    {"Cfish_Obj_Inc_RefCount", "Lucy_Obj_Inc_RefCount"},
    {"Cfish_Obj_Is_A", "Lucy_Obj_Is_A"},
    {"Cfish_Obj_Load", "Lucy_Obj_Load"},
    {"Cfish_Obj_Load_OFFSET", "Lucy_Obj_Load_OFFSET"},
    {"Cfish_Obj_Load_t", "Lucy_Obj_Load_t"},
    {"Cfish_Obj_Serialize", "Lucy_Obj_Serialize"},
    {"Cfish_Obj_To_F64", "Lucy_Obj_To_F64"},
    {"Cfish_Obj_To_I64", "Lucy_Obj_To_I64"},
    {"Cfish_Obj_To_Bool", "Lucy_Obj_To_Bool"},
    {"Cfish_Obj_To_Host", "Lucy_Obj_To_Host"},

    {"cfish_ByteBuf", "lucy_ByteBuf"},
    {"CFISH_BYTEBUF", "LUCY_BYTEBUF"},
    {"cfish_BB_init", "lucy_BB_init"},
    {"Cfish_BB_Deserialize", "Lucy_BB_Deserialize"},
    {"Cfish_BB_Get_Buf", "Lucy_BB_Get_Buf"},
    {"Cfish_BB_Get_Size", "Lucy_BB_Get_Size"},
    {"Cfish_BB_Mimic_Bytes", "Lucy_BB_Mimic_Bytes"},
    {"Cfish_BB_To_Host", "Lucy_BB_To_Host"},
    {"cfish_ViewByteBuf", "lucy_ViewByteBuf"},
    {"cfish_ViewBB_new", "lucy_ViewBB_new"},
    {"Cfish_ViewBB_Assign_Bytes", "Lucy_ViewBB_Assign_Bytes"},

    {"cfish_CharBuf", "lucy_CharBuf"},
    {"CFISH_CHARBUF", "LUCY_CHARBUF"},
    {"cfish_CB_clone", "lucy_CB_clone"},
    {"cfish_CB_newf", "lucy_CB_newf"},
    {"cfish_CB_new_from_trusted_utf8", "lucy_CB_new_from_trusted_utf8"},
    {"cfish_CB_init", "lucy_CB_init"},
    {"Cfish_CB_Cat_Trusted_Str", "Lucy_CB_Cat_Trusted_Str"},
    {"Cfish_CB_Clone", "Lucy_CB_Clone"},
    {"Cfish_CB_Deserialize", "Lucy_CB_Deserialize"},
    {"Cfish_CB_Find_Str", "Lucy_CB_Find_Str"},
    {"Cfish_CB_Get_Ptr8", "Lucy_CB_Get_Ptr8"},
    {"Cfish_CB_Get_Size", "Lucy_CB_Get_Size"},
    {"Cfish_CB_Mimic_Str", "Lucy_CB_Mimic_Str"},
    {"Cfish_CB_Nip_One_OFFSET", "Lucy_CB_Nip_One_OFFSET"},
    {"Cfish_CB_Nip_One_t", "Lucy_CB_Nip_One_t"},
    {"Cfish_CB_To_Host", "Lucy_CB_To_Host"},
    {"cfish_ViewCharBuf", "lucy_ViewCharBuf"},
    {"CFISH_VIEWCHARBUF", "LUCY_VIEWCHARBUF"},
    {"cfish_ViewCB_new_from_trusted_utf8", "lucy_ViewCB_new_from_trusted_utf8"},
    {"Cfish_ViewCB_Assign_Str", "Lucy_ViewCB_Assign_Str"},
    {"cfish_ZombieCharBuf", "lucy_ZombieCharBuf"},
    {"CFISH_ZOMBIECHARBUF", "LUCY_ZOMBIECHARBUF"},
    {"cfish_ZCB_new", "lucy_ZCB_new"},
    {"cfish_ZCB_size", "lucy_ZCB_size"},
    {"cfish_ZCB_wrap", "lucy_ZCB_wrap"},
    {"cfish_ZCB_wrap_str", "lucy_ZCB_wrap_str"},
    {"Cfish_ZCB_Assign_Str", "Lucy_ZCB_Assign_Str"},
    {"Cfish_ZCB_Assign_Trusted_Str", "Lucy_ZCB_Assign_Trusted_Str"},

    {"CFISH_FLOATNUM", "LUCY_FLOATNUM"},
    {"CFISH_INTNUM", "LUCY_INTNUM"},
    {"CFISH_INTEGER32", "LUCY_INTEGER32"},
    {"CFISH_INTEGER64", "LUCY_INTEGER64"},
    {"CFISH_FLOAT32", "LUCY_FLOAT32"},
    {"CFISH_FLOAT64", "LUCY_FLOAT64"},
    {"cfish_Bool_false_singleton", "lucy_Bool_false_singleton"},
    {"cfish_Bool_true_singleton", "lucy_Bool_true_singleton"},
    {"cfish_Bool_singleton", "lucy_Bool_singleton"},
    {"Cfish_Bool_Dec_RefCount_OFFSET", "Lucy_Bool_Dec_RefCount_OFFSET"},
    {"Cfish_Bool_Dec_RefCount_t", "Lucy_Bool_Dec_RefCount_t"},

    {"cfish_Err", "lucy_Err"},
    {"CFISH_ERR", "LUCY_ERR"},
    {"cfish_Err_certify", "lucy_Err_certify"},
    {"cfish_Err_do_throw", "lucy_Err_do_throw"},
    {"cfish_Err_downcast", "lucy_Err_downcast"},
    {"cfish_Err_get_error", "lucy_Err_get_error"},
    {"cfish_Err_init_class", "lucy_Err_init_class"},
    {"cfish_Err_make_mess", "lucy_Err_make_mess"},
    {"cfish_Err_new", "lucy_Err_new"},
    {"cfish_Err_rethrow", "lucy_Err_rethrow"},
    {"cfish_Err_set_error", "lucy_Err_set_error"},
    {"cfish_Err_throw_at", "lucy_Err_throw_at"},
    {"cfish_Err_throw_mess", "lucy_Err_throw_mess"},
    {"cfish_Err_to_host", "lucy_Err_to_host"},
    {"cfish_Err_trap", "lucy_Err_trap"},
    {"cfish_Err_warn_at", "lucy_Err_warn_at"},
    {"cfish_Err_warn_mess", "lucy_Err_warn_mess"},
    {"Cfish_Err_Add_Frame", "Lucy_Err_Add_Frame"},
    {"Cfish_Err_Cat_Mess", "Lucy_Err_Cat_Mess"},
    {"Cfish_Err_Make_OFFSET", "Lucy_Err_Make_OFFSET"},
    {"Cfish_Err_Make_t", "Lucy_Err_Make_t"},
    {"Cfish_Err_To_Host", "Lucy_Err_To_Host"},
    {"Cfish_Err_To_Host_OFFSET", "Lucy_Err_To_Host_OFFSET"},
    {"Cfish_Err_To_Host_t", "Lucy_Err_To_Host_t"},

    {"cfish_Hash", "lucy_Hash"},
    {"CFISH_HASH", "LUCY_HASH"},
    {"cfish_Hash_new", "lucy_Hash_new"},
    {"cfish_Hash_fetch", "lucy_Hash_fetch"},
    {"cfish_Hash_store", "lucy_Hash_store"},
    {"Cfish_Hash_Deserialize", "Lucy_Hash_Deserialize"},
    {"Cfish_Hash_Fetch_Str", "Lucy_Hash_Fetch_Str"},
    {"Cfish_Hash_Iterate", "Lucy_Hash_Iterate"},
    {"Cfish_Hash_Next", "Lucy_Hash_Next"},
    {"Cfish_Hash_Store_Str", "Lucy_Hash_Store_Str"},
    {"Cfish_Hash_Store", "Lucy_Hash_Store"},
    {"Cfish_Hash_To_Host", "Lucy_Hash_To_Host"},

    {"cfish_LockFreeRegistry", "lucy_LockFreeRegistry"},
    {"CFISH_LOCKFREEREGISTRY", "LUCY_LOCKFREEREGISTRY"},
    {"cfish_LFReg_to_host", "lucy_LFReg_to_host"},
    {"Cfish_LFReg_To_Host", "Lucy_LFReg_To_Host"},
    {"Cfish_LFReg_To_Host_OFFSET", "Lucy_LFReg_To_Host_OFFSET"},
    {"Cfish_LFReg_To_Host_t", "Lucy_LFReg_To_Host_t"},

    {"cfish_Memory_wrapped_calloc", "lucy_Memory_wrapped_calloc"},
    {"cfish_Memory_wrapped_free", "lucy_Memory_wrapped_free"},
    {"cfish_Memory_wrapped_malloc", "lucy_Memory_wrapped_malloc"},
    {"cfish_Memory_wrapped_realloc", "lucy_Memory_wrapped_realloc"},

    {"cfish_Float32", "lucy_Float32"},
    {"cfish_Float32_init", "lucy_Float32_init"},
    {"Cfish_Float32_Set_Value", "Lucy_Float32_Set_Value"},
    {"cfish_Float64", "lucy_Float64"},
    {"cfish_Float64_init", "lucy_Float64_init"},
    {"Cfish_Float64_Set_Value", "Lucy_Float64_Set_Value"},
    {"cfish_Integer32", "lucy_Integer32"},
    {"Cfish_Int32_Set_Value", "Lucy_Int32_Set_Value"},
    {"cfish_Integer64", "lucy_Integer64"},
    {"Cfish_Int64_Set_Value", "Lucy_Int64_Set_Value"},

    {"cfish_NumUtil_decode_bigend_f32", "lucy_NumUtil_decode_bigend_f32"},
    {"cfish_NumUtil_decode_bigend_f64", "lucy_NumUtil_decode_bigend_f64"},
    {"cfish_NumUtil_decode_bigend_u16", "lucy_NumUtil_decode_bigend_u16"},
    {"cfish_NumUtil_decode_bigend_u32", "lucy_NumUtil_decode_bigend_u32"},
    {"cfish_NumUtil_decode_bigend_u64", "lucy_NumUtil_decode_bigend_u64"},
    {"cfish_NumUtil_decode_c32", "lucy_NumUtil_decode_c32"},
    {"cfish_NumUtil_decode_c64", "lucy_NumUtil_decode_c64"},
    {"cfish_NumUtil_encode_bigend_f32", "lucy_NumUtil_encode_bigend_f32"},
    {"cfish_NumUtil_encode_bigend_f64", "lucy_NumUtil_encode_bigend_f64"},
    {"cfish_NumUtil_encode_bigend_u16", "lucy_NumUtil_encode_bigend_u16"},
    {"cfish_NumUtil_encode_bigend_u32", "lucy_NumUtil_encode_bigend_u32"},
    {"cfish_NumUtil_encode_bigend_u64", "lucy_NumUtil_encode_bigend_u64"},
    {"cfish_NumUtil_encode_c32", "lucy_NumUtil_encode_c32"},
    {"cfish_NumUtil_encode_c64", "lucy_NumUtil_encode_c64"},
    {"cfish_NumUtil_encode_padded_c32", "lucy_NumUtil_encode_padded_c32"},
    {"cfish_NumUtil_u1clear", "lucy_NumUtil_u1clear"},
    {"cfish_NumUtil_u1flip", "lucy_NumUtil_u1flip"},
    {"cfish_NumUtil_u1get", "lucy_NumUtil_u1get"},
    {"cfish_NumUtil_u1set", "lucy_NumUtil_u1set"},
    {"cfish_NumUtil_u1masks", "lucy_NumUtil_u1masks"},
    {"cfish_NumUtil_u2get", "lucy_NumUtil_u2get"},
    {"cfish_NumUtil_u2set", "lucy_NumUtil_u2set"},
    {"cfish_NumUtil_u2masks", "lucy_NumUtil_u2masks"},
    {"cfish_NumUtil_u2shifts", "lucy_NumUtil_u2shifts"},
    {"cfish_NumUtil_u4get", "lucy_NumUtil_u4get"},
    {"cfish_NumUtil_u4set", "lucy_NumUtil_u4set"},
    {"cfish_NumUtil_u4masks", "lucy_NumUtil_u4masks"},
    {"cfish_NumUtil_u4shifts", "lucy_NumUtil_u4shifts"},
    {"cfish_NumUtil_skip_cint", "lucy_NumUtil_skip_cint"},

    {"cfish_StrHelp_to_base36", "lucy_StrHelp_to_base36"},
    {"cfish_StrHelp_utf8_valid", "lucy_StrHelp_utf8_valid"},
    {"cfish_StrHelp_UTF8_COUNT", "lucy_StrHelp_UTF8_COUNT"},

    {"cfish_TestBatch_fail", "lucy_TestBatch_fail"},
    {"cfish_TestBatch_pass", "lucy_TestBatch_pass"},
    {"cfish_TestBatch_skip", "lucy_TestBatch_skip"},
    {"cfish_TestBatch_test_false", "lucy_TestBatch_test_false"},
    {"cfish_TestBatch_test_float_equals", "lucy_TestBatch_test_float_equals"},
    {"cfish_TestBatch_test_int_equals", "lucy_TestBatch_test_int_equals"},
    {"cfish_TestBatch_test_string_equals", "lucy_TestBatch_test_string_equals"},
    {"cfish_TestBatch_test_true", "lucy_TestBatch_test_true"},
    {"cfish_TestFormatter", "lucy_TestFormatter"},
    {"cfish_TestFormatterCF", "lucy_TestFormatterCF"},
    {"cfish_TestFormatterCF_new", "lucy_TestFormatterCF_new"},
    {"cfish_TestFormatterTAP", "lucy_TestFormatterTAP"},
    {"cfish_TestFormatterTAP_new", "lucy_TestFormatterTAP_new"},

    {"cfish_VArray", "lucy_VArray"},
    {"CFISH_VARRAY", "LUCY_VARRAY"},
    {"cfish_VA_new", "lucy_VA_new"},
    {"cfish_VA_store", "lucy_VA_store"},
    {"Cfish_VA_Clone", "Lucy_VA_Clone"},
    {"Cfish_VA_Delete", "Lucy_VA_Delete"},
    {"Cfish_VA_Deserialize", "Lucy_VA_Deserialize"},
    {"Cfish_VA_Fetch", "Lucy_VA_Fetch"},
    {"Cfish_VA_Get_Size", "Lucy_VA_Get_Size"},
    {"Cfish_VA_Pop", "Lucy_VA_Pop"},
    {"Cfish_VA_Push", "Lucy_VA_Push"},
    {"Cfish_VA_Resize", "Lucy_VA_Resize"},
    {"Cfish_VA_Shallow_Copy", "Lucy_VA_Shallow_Copy"},
    {"Cfish_VA_Shift", "Lucy_VA_Shift"},
    {"Cfish_VA_Store", "Lucy_VA_Store"},
    {"Cfish_VA_To_Host", "Lucy_VA_To_Host"},

    {"cfish_VTable", "lucy_VTable"},
    {"CFISH_VTABLE", "LUCY_VTABLE"},
    {"cfish_VTable_add_alias_to_registry", "lucy_VTable_add_alias_to_registry"},
    {"cfish_VTable_bootstrap", "lucy_VTable_bootstrap"},
    {"cfish_VTable_fetch_vtable", "lucy_VTable_fetch_vtable"},
    {"cfish_VTable_find_parent_class", "lucy_VTable_find_parent_class"},
    {"cfish_VTable_foster_obj", "lucy_VTable_foster_obj"},
    {"cfish_VTable_fresh_host_methods", "lucy_VTable_fresh_host_methods"},
    {"cfish_VTable_init_obj", "lucy_VTable_init_obj"},
    {"cfish_VTable_init_registry", "lucy_VTable_init_registry"},
    {"cfish_VTable_make_obj", "lucy_VTable_make_obj"},
    {"cfish_VTable_offset_of_parent", "lucy_VTable_offset_of_parent"},
    {"cfish_VTable_register_with_host", "lucy_VTable_register_with_host"},
    {"cfish_VTable_registry", "lucy_VTable_registry"},
    {"cfish_VTable_singleton", "lucy_VTable_singleton"},
    {"cfish_VTable_to_host", "lucy_VTable_to_host"},
    {"Cfish_VTable_Foster_Obj", "Lucy_VTable_Foster_Obj"},
    {"Cfish_VTable_Get_Name", "Lucy_VTable_Get_Name"},
    {"Cfish_VTable_Make_Obj", "Lucy_VTable_Make_Obj"},
    {"Cfish_VTable_To_Host", "Lucy_VTable_To_Host"},
    {"Cfish_VTable_To_Host_OFFSET", "Lucy_VTable_To_Host_OFFSET"},
    {"Cfish_VTable_To_Host_t", "Lucy_VTable_To_Host_t"},

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

