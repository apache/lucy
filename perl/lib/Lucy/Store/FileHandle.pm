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

package Lucy::Store::FileHandle;
use Lucy;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy     PACKAGE = Lucy::Store::FileHandle

=for comment

For testing purposes only.  Track number of FileHandle objects in existence.

=cut

uint32_t
FH_READ_ONLY()
CODE:
    RETVAL = LUCY_FH_READ_ONLY;
OUTPUT: RETVAL

uint32_t
FH_WRITE_ONLY()
CODE:
    RETVAL = LUCY_FH_WRITE_ONLY;
OUTPUT: RETVAL

uint32_t
FH_CREATE()
CODE:
    RETVAL = LUCY_FH_CREATE;
OUTPUT: RETVAL

uint32_t
FH_EXCLUSIVE()
CODE:
    RETVAL = LUCY_FH_EXCLUSIVE;
OUTPUT: RETVAL


int32_t
object_count()
CODE:
    RETVAL = lucy_FH_object_count;
OUTPUT: RETVAL

=for comment

For testing purposes only.  Used to help produce buffer alignment tests.

=cut

IV
_BUF_SIZE()
CODE:
   RETVAL = LUCY_IO_STREAM_BUF_SIZE;
OUTPUT: RETVAL
END_XS_CODE

Clownfish::Binding::Perl::Class->register(
    parcel            => "Lucy",
    class_name        => "Lucy::Store::FileHandle",
    xs_code           => $xs_code,
    bind_methods      => [qw( Length Close )],
    bind_constructors => ['_open|do_open'],
);


