use Lucy;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE =  Lucy    PACKAGE = Lucy::Object::Hash

SV*
_fetch(self, key)
    lucy_Hash *self;
    lucy_ZombieCharBuf key;
CODE:
    RETVAL = LUCY_OBJ_TO_SV(lucy_Hash_fetch(self, (lucy_Obj*)&key));
OUTPUT: RETVAL

void
store(self, key, value);
    lucy_Hash          *self; 
    lucy_ZombieCharBuf  key;
    lucy_Obj           *value;
PPCODE:
{
    if (value) { LUCY_INCREF(value); }
    lucy_Hash_store(self, (lucy_Obj*)&key, value);
}

void
iter_next(self)
    lucy_Hash *self;
PPCODE:
{
    lucy_Obj *key;
    lucy_Obj *val;

    if (Lucy_Hash_Iter_Next(self, &key, &val)) {
        SV *key_sv = Lucy_Obj_To_Host(key);
        SV *val_sv = Lucy_Obj_To_Host(val);

        XPUSHs(sv_2mortal( key_sv ));
        XPUSHs(sv_2mortal( val_sv ));
        XSRETURN(2);
    }
    else {
        XSRETURN_EMPTY;
    }
}
END_XS_CODE

Boilerplater::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Object::Hash",
    xs_code      => $xs_code,
    bind_methods => [
        qw(
            Fetch
            Delete
            Keys
            Values
            Find_Key
            Clear
            Iter_Init
            Get_Size
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

