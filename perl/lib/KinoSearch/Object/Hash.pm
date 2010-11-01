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

package KinoSearch::Object::Hash;
use KinoSearch;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE =  KinoSearch    PACKAGE = KinoSearch::Object::Hash

SV*
_deserialize(either_sv, instream)
    SV *either_sv;
    kino_InStream *instream;
CODE:
    CHY_UNUSED_VAR(either_sv);
    RETVAL = CFISH_OBJ_TO_SV_NOINC(kino_Hash_deserialize(NULL, instream));
OUTPUT: RETVAL

SV*
_fetch(self, key)
    kino_Hash *self;
    const kino_CharBuf *key;
CODE:
    RETVAL = CFISH_OBJ_TO_SV(kino_Hash_fetch(self, (kino_Obj*)key));
OUTPUT: RETVAL

void
store(self, key, value);
    kino_Hash          *self; 
    const kino_CharBuf *key;
    kino_Obj           *value;
PPCODE:
{
    if (value) { LUCY_INCREF(value); }
    kino_Hash_store(self, (kino_Obj*)key, value);
}

void
next(self)
    kino_Hash *self;
PPCODE:
{
    kino_Obj *key;
    kino_Obj *val;

    if (Kino_Hash_Next(self, &key, &val)) {
        SV *key_sv = (SV*)Kino_Obj_To_Host(key);
        SV *val_sv = (SV*)Kino_Obj_To_Host(val);

        XPUSHs(sv_2mortal( key_sv ));
        XPUSHs(sv_2mortal( val_sv ));
        XSRETURN(2);
    }
    else {
        XSRETURN_EMPTY;
    }
}
END_XS_CODE

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Object::Hash",
    xs_code      => $xs_code,
    bind_methods => [
        qw(
            Fetch
            Delete
            Keys
            Values
            Find_Key
            Clear
            Iterate
            Get_Size
            )
    ],
    bind_constructors => ["new"],
);


