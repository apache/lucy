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

package Lucy::Store::OutStream;
use Lucy;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy     PACKAGE = Lucy::Store::OutStream

void
print(self, ...)
    lucy_OutStream *self;
PPCODE:
{
    int i;
    for (i = 1; i < items; i++) {
        STRLEN len;
        char *ptr = SvPV(ST(i), len);
        Lucy_OutStream_Write_Bytes(self, ptr, len);
    }
}

void
write_string(self, aSV)
    lucy_OutStream *self;
    SV *aSV;
PPCODE:
{
    STRLEN len = 0;
    char *ptr = SvPVutf8(aSV, len);
    Lucy_OutStream_Write_C32(self, len);
    Lucy_OutStream_Write_Bytes(self, ptr, len);
}
END_XS_CODE

my $synopsis = <<'END_SYNOPSIS';    # Don't use this yet.
    my $outstream = $folder->open_out($filename) or die $@;
    $outstream->write_u64($file_position);
END_SYNOPSIS

Clownfish::CFC::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Store::OutStream",
    xs_code      => $xs_code,
    bind_methods => [
        qw(
            Tell
            Length
            Flush
            Close
            Absorb
            Write_I8
            Write_I32
            Write_I64
            Write_U8
            Write_U32
            Write_U64
            Write_C32
            Write_C64
            Write_F32
            Write_F64
            )
    ],
    bind_constructors => ['open|do_open'],
);


