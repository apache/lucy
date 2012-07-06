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

package Lucy::Store::InStream;
use Lucy;
our $VERSION = '0.003002';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy    PACKAGE = Lucy::Store::InStream

void
read(self, buffer_sv, len, ...)
    lucy_InStream *self;
    SV *buffer_sv;
    size_t len;
PPCODE:
{
    UV offset = items == 4 ? SvUV(ST(3)) : 0;
    char *ptr;
    size_t total_len = offset + len;
    SvUPGRADE(buffer_sv, SVt_PV);
    if (!SvPOK(buffer_sv)) { SvCUR_set(buffer_sv, 0); }
    ptr = SvGROW(buffer_sv, total_len + 1);
    Lucy_InStream_Read_Bytes(self, ptr + offset, len);
    SvPOK_on(buffer_sv);
    if (SvCUR(buffer_sv) < total_len) {
        SvCUR_set(buffer_sv, total_len);
        *(SvEND(buffer_sv)) = '\0';
    }
}

SV*
read_string(self)
    lucy_InStream *self;
CODE:
{
    char *ptr;
    size_t len = Lucy_InStream_Read_C32(self);
    RETVAL = newSV(len + 1);
    SvCUR_set(RETVAL, len);
    SvPOK_on(RETVAL);
    SvUTF8_on(RETVAL); // Trust source.  Reconsider if API goes public.
    *SvEND(RETVAL) = '\0';
    ptr = SvPVX(RETVAL);
    Lucy_InStream_Read_Bytes(self, ptr, len);
}
OUTPUT: RETVAL

int
read_raw_c64(self, buffer_sv)
    lucy_InStream *self;
    SV *buffer_sv;
CODE:
{
    char *ptr;
    SvUPGRADE(buffer_sv, SVt_PV);
    ptr = SvGROW(buffer_sv, 10 + 1);
    RETVAL = Lucy_InStream_Read_Raw_C64(self, ptr);
    SvPOK_on(buffer_sv);
    SvCUR_set(buffer_sv, RETVAL);
}
OUTPUT: RETVAL
END_XS_CODE

Clownfish::CFC::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Store::InStream",
    xs_code      => $xs_code,
    bind_methods => [
        qw(
            Seek
            Tell
            Length
            Reopen
            Close
            Read_I8
            Read_I32
            Read_I64
            Read_U8
            Read_U32
            Read_U64
            Read_C32
            Read_C64
            Read_F32
            Read_F64
            )
    ],
    bind_constructors => ['open|do_open'],
);


