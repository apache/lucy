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

package Lucy::Object::ByteBuf;
use Lucy;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy     PACKAGE = Lucy::Object::ByteBuf

SV*
new(either_sv, sv)
    SV *either_sv;
    SV *sv;
CODE:
{
    STRLEN size;
    char *ptr = SvPV(sv, size);
    lucy_ByteBuf *self = (lucy_ByteBuf*)XSBind_new_blank_obj(either_sv);
    lucy_BB_init(self, size);
    Lucy_BB_Mimic_Bytes(self, ptr, size);
    RETVAL = CFISH_OBJ_TO_SV_NOINC(self);
}
OUTPUT: RETVAL

SV*
_deserialize(either_sv, instream)
    SV *either_sv;
    lucy_InStream *instream;
CODE:
    CHY_UNUSED_VAR(either_sv);
    RETVAL = CFISH_OBJ_TO_SV_NOINC(lucy_BB_deserialize(NULL, instream));
OUTPUT: RETVAL
END_XS_CODE

Clownfish::CFC::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Object::ByteBuf",
    xs_code      => $xs_code,
    bind_methods => [
        qw(
            Get_Size
            Get_Capacity
            Cat
            )
    ],
);


