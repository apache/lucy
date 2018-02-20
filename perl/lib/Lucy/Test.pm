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

package Lucy::Test;
use Lucy;
our $VERSION = '0.006002';
$VERSION = eval $VERSION;

sub dl_load_flags { 1 }

BEGIN {
    require DynaLoader;
    our @ISA = qw( DynaLoader );
    bootstrap Lucy::Test '0.6.2';
}

# Set the default memory threshold for PostingListWriter to a low number so
# that we simulate large indexes by performing a lot of PostingPool flushes.
Lucy::Index::PostingListWriter::set_default_mem_thresh(0x1000);

1;

__END__


