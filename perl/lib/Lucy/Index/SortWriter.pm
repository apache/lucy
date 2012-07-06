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

package Lucy::Index::SortWriter;
use Lucy;
our $VERSION = '0.003002';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS';
MODULE = Lucy    PACKAGE = Lucy::Index::SortWriter

void
set_default_mem_thresh(mem_thresh)
    size_t mem_thresh;
PPCODE:
    lucy_SortWriter_set_default_mem_thresh(mem_thresh);
END_XS

Clownfish::CFC::Binding::Perl::Class->register(
    parcel            => "Lucy",
    class_name        => "Lucy::Index::SortWriter",
    xs_code           => $xs_code,
    bind_constructors => ["new"],
);


