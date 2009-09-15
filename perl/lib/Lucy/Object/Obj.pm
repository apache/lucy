use Lucy;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy     PACKAGE = Lucy::Object::Obj

chy_bool_t
is_a(self, class_name)
    lucy_Obj *self;
    lucy_ZombieCharBuf class_name;
CODE:
{
    lucy_VTable *target = lucy_VTable_fetch_vtable((lucy_CharBuf*)&class_name);
    RETVAL = Lucy_Obj_Is_A(self, target);
}
OUTPUT: RETVAL

void
DESTROY(self)
    lucy_Obj *self;
PPCODE:
    /*
    {
        char *perl_class = HvNAME(SvSTASH(SvRV(ST(0))));
        warn("Destroying: 0x%x %s", (unsigned)self, perl_class);
    }
    */
    Lucy_Obj_Destroy(self);
END_XS_CODE

Boilerplater::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Object::Obj",
    xs_code      => $xs_code,
    bind_methods => [
        qw(
            Get_RefCount
            Inc_RefCount
            Dec_RefCount
            Get_VTable
            To_String
            To_I64
            To_F64
            Dump
            _load|Load
            Clone
            Mimic
            Equals
            Hash_Code
            Destroy
            )
    ],
    bind_constructors => ["new"],
);

__COPYRIGHT__

    /**
     * Copyright 2009 The Apache Software Foundation
     *
     * Licensed under the Apache License, Version 2.0 (the "License");
     * you may not use this file except in compliance with the License.
     * You may obtain a copy of the License at
     *
     *     http://www.apache.org/licenses/LICENSE-2.0
     *
     * Unless required by applicable law or agreed to in writing, software
     * distributed under the License is distributed on an "AS IS" BASIS,
     * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
     * implied.  See the License for the specific language governing
     * permissions and limitations under the License.
     */

