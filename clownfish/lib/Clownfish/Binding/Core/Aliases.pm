# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

use strict;
use warnings;

package Clownfish::Binding::Core::Aliases;

our %aliases = (
    cfish_ref_t        => 'kino_ref_t',
    cfish_method_t     => 'kino_method_t',
    cfish_method       => 'kino_method',
    cfish_super_method => 'kino_super_method',

    cfish_Obj                => 'kino_Obj',
    CFISH_OBJ                => 'KINO_OBJ',
    Cfish_Obj_Dump           => 'Kino_Obj_Dump',
    Cfish_Obj_Get_Class_Name => 'Kino_Obj_Get_Class_Name',
    Cfish_Obj_Is_A           => 'Kino_Obj_Is_A',
    Cfish_Obj_Load           => 'Kino_Obj_Load',
    Cfish_Obj_To_F64         => 'Kino_Obj_To_F64',
    Cfish_Obj_To_I64         => 'Kino_Obj_To_I64',
    Cfish_Obj_To_Host        => 'Kino_Obj_To_Host',
    Cfish_Obj_Dec_RefCount   => 'Kino_Obj_Dec_RefCount',
    Cfish_Obj_Inc_RefCount   => 'Kino_Obj_Inc_RefCount',

    cfish_ByteBuf     => 'kino_ByteBuf',
    CFISH_BYTEBUF     => 'KINO_BYTEBUF',
    Cfish_BB_Get_Size => 'Kino_BB_Get_Size',
    Cfish_BB_Get_Buf  => 'Kino_BB_Get_Buf',

    cfish_CharBuf                  => 'kino_CharBuf',
    CFISH_CHARBUF                  => 'KINO_CHARBUF',
    cfish_CB_newf                  => 'kino_CB_newf',
    cfish_CB_new_from_trusted_utf8 => 'kino_CB_new_from_trusted_utf8',
    Cfish_CB_Clone                 => 'Kino_CB_Clone',
    cfish_ZombieCharBuf            => 'kino_ZombieCharBuf',
    CFISH_ZOMBIECHARBUF            => 'KINO_ZOMBIECHARBUF',
    CFISH_VIEWCHARBUF              => 'KINO_VIEWCHARBUF',
    cfish_ZCB_size                 => 'kino_ZCB_size',
    cfish_ZCB_wrap_str             => 'kino_ZCB_wrap_str',
    Cfish_CB_Get_Ptr8              => 'Kino_CB_Get_Ptr8',
    Cfish_CB_Get_Size              => 'Kino_CB_Get_Size',

    CFISH_FLOATNUM  => 'KINO_FLOATNUM',
    CFISH_INTNUM    => 'KINO_INTNUM',
    CFISH_INTEGER32 => 'KINO_INTEGER32',
    CFISH_INTEGER64 => 'KINO_INTEGER64',
    CFISH_FLOAT32   => 'KINO_FLOAT32',
    CFISH_FLOAT64   => 'KINO_FLOAT64',

    CFISH_ERR => 'KINO_ERR',

    cfish_Hash           => 'kino_Hash',
    CFISH_HASH           => 'KINO_HASH',
    cfish_Hash_new       => 'kino_Hash_new',
    Cfish_Hash_Iterate   => 'Kino_Hash_Iterate',
    Cfish_Hash_Next      => 'Kino_Hash_Next',
    Cfish_Hash_Fetch_Str => 'Kino_Hash_Fetch_Str',
    Cfish_Hash_Store_Str => 'Kino_Hash_Store_Str',

    cfish_VArray      => 'kino_VArray',
    CFISH_VARRAY      => 'KINO_VARRAY',
    cfish_VA_new      => 'kino_VA_new',
    Cfish_VA_Fetch    => 'Kino_VA_Fetch',
    Cfish_VA_Get_Size => 'Kino_VA_Get_Size',
    Cfish_VA_Resize   => 'Kino_VA_Resize',
    Cfish_VA_Store    => 'Kino_VA_Store',

    cfish_VTable                  => 'kino_VTable',
    CFISH_VTABLE                  => 'KINO_VTABLE',
    cfish_VTable_add_to_registry  => 'kino_VTable_add_to_registry',
    cfish_VTable_offset_of_parent => 'kino_VTable_offset_of_parent',
    cfish_VTable_singleton        => 'kino_VTable_singleton',
    Cfish_VTable_Get_Name         => 'Kino_VTable_Get_Name',
    Cfish_VTable_Make_Obj         => 'Kino_VTable_Make_Obj',

    cfish_Host_callback      => 'kino_Host_callback',
    cfish_Host_callback_f64  => 'kino_Host_callback_f64',
    cfish_Host_callback_host => 'kino_Host_callback_host',
    cfish_Host_callback_i64  => 'kino_Host_callback_i64',
    cfish_Host_callback_obj  => 'kino_Host_callback_obj',
    cfish_Host_callback_str  => 'kino_Host_callback_str',

    CFISH_USE_SHORT_NAMES => 'LUCY_USE_SHORT_NAMES',
);

sub c_aliases {
    my $content = "#ifndef CFISH_C_ALIASES\n#define CFISH_C_ALIASES\n\n";
    for my $alias ( keys %aliases ) {
        $content .= "#define $alias $aliases{$alias}\n";
    }
    $content .= "\n#endif /* CFISH_C_ALIASES */\n\n";
    return $content;
}

1;

