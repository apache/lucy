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

package Lucy::Object::VTable;
use Lucy;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Object::VTable

SV*
_get_registry()
CODE:
    if (lucy_VTable_registry == NULL) {
        lucy_VTable_init_registry();
    }
    RETVAL = (SV*)Lucy_Obj_To_Host((lucy_Obj*)lucy_VTable_registry);
OUTPUT: RETVAL

SV*
singleton(unused_sv, ...)
    SV *unused_sv;
CODE:
{
    CHY_UNUSED_VAR(unused_sv);
    lucy_CharBuf *class_name = NULL;
    lucy_VTable  *parent     = NULL;
    chy_bool_t args_ok
        = XSBind_allot_params(&(ST(0)), 1, items,
                              "Lucy::Object::VTable::singleton_PARAMS",
                              ALLOT_OBJ(&class_name, "class_name", 10, true,
                                        LUCY_CHARBUF, alloca(cfish_ZCB_size())),
                              ALLOT_OBJ(&parent, "parent", 6, false,
                                        LUCY_VTABLE, NULL),
                              NULL);
    if (!args_ok) {
        CFISH_RETHROW(CFISH_INCREF(cfish_Err_get_error()));
    }
    lucy_VTable *singleton = lucy_VTable_singleton(class_name, parent);
    RETVAL = (SV*)Lucy_VTable_To_Host(singleton);
}
OUTPUT: RETVAL

SV*
make_obj(self)
    lucy_VTable *self;
CODE:
    lucy_Obj *blank = Lucy_VTable_Make_Obj(self);
    RETVAL = CFISH_OBJ_TO_SV_NOINC(blank);
OUTPUT: RETVAL
END_XS_CODE

Clownfish::CFC::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Object::VTable",
    xs_code      => $xs_code,
    bind_methods => [qw( Get_Name Get_Parent )],
);


