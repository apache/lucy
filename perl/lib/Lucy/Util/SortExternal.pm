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

package Lucy::Util::SortExternal;
use Lucy;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy    PACKAGE = Lucy::Util::SortExternal

IV
_DEFAULT_MEM_THRESHOLD()
CODE:
    RETVAL = LUCY_SORTEX_DEFAULT_MEM_THRESHOLD;
OUTPUT: RETVAL
END_XS_CODE

Clownfish::CFC::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Util::SortExternal",
    xs_code      => $xs_code,
    bind_methods => [
        qw(
            Flush
            Flip
            Add_Run
            Refill
            Sort_Cache
            Cache_Count
            Clear_Cache
            Set_Mem_Thresh
            )
    ],
);


