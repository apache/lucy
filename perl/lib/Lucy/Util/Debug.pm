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

package Lucy::Util::Debug;
use strict;
use warnings;

use Lucy;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Util::Debug

#include "Lucy/Util/Debug.h"

void
DEBUG_PRINT(message)
    char *message;
PPCODE:
    LUCY_DEBUG_PRINT("%s", message);

void
DEBUG(message)
    char *message;
PPCODE:
    LUCY_DEBUG("%s", message);

chy_bool_t
DEBUG_ENABLED()
CODE:
    RETVAL = LUCY_DEBUG_ENABLED;
OUTPUT: RETVAL

=for comment

Keep track of any Lucy objects that have been assigned to global Perl
variables.  This is useful when accounting how many objects should have been
destroyed and diagnosing memory leaks.

=cut

void
track_globals(...)
PPCODE:
{
    CHY_UNUSED_VAR(items);
    LUCY_IFDEF_DEBUG(lucy_Debug_num_globals++;);
}

void
set_env_cache(str)
    char *str;
PPCODE:
    lucy_Debug_set_env_cache(str);

void
ASSERT(maybe)
    int maybe;
PPCODE:
    LUCY_ASSERT(maybe, "XS ASSERT binding test");

IV
num_allocated()
CODE:
    RETVAL = lucy_Debug_num_allocated;
OUTPUT: RETVAL

IV
num_freed()
CODE:
    RETVAL = lucy_Debug_num_freed;
OUTPUT: RETVAL

IV
num_globals()
CODE:
    RETVAL = lucy_Debug_num_globals;
OUTPUT: RETVAL
END_XS_CODE

Clownfish::Binding::Perl::Class->register(
    parcel     => "Lucy",
    class_name => "Lucy::Util::Debug",
    xs_code    => $xs_code,
);


