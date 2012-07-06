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

package Lucy::Test::Util::BBSortEx;
use Lucy;
our $VERSION = '0.003002';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy    PACKAGE = Lucy::Test::Util::BBSortEx

SV*
fetch(self)
    lucy_BBSortEx *self;
CODE:
{
    void *address = Lucy_BBSortEx_Fetch(self);
    if (address) {
        RETVAL = XSBind_cfish_to_perl(*(lucy_Obj**)address);
        CFISH_DECREF(*(lucy_Obj**)address);
    }
    else {
        RETVAL = newSV(0);
    }
}
OUTPUT: RETVAL

SV*
peek(self)
    lucy_BBSortEx *self;
CODE:
{
    void *address = Lucy_BBSortEx_Peek(self);
    if (address) {
        RETVAL = XSBind_cfish_to_perl(*(lucy_Obj**)address);
    }
    else {
        RETVAL = newSV(0);
    }
}
OUTPUT: RETVAL

void
feed(self, bb)
    lucy_BBSortEx *self;
    lucy_ByteBuf *bb;
CODE:
    CFISH_INCREF(bb);
    Lucy_BBSortEx_Feed(self, &bb);

END_XS_CODE

Clownfish::CFC::Binding::Perl::Class->register(
    parcel            => "Lucy",
    class_name        => "Lucy::Test::Util::BBSortEx",
    bind_constructors => ["new"],
    xs_code           => $xs_code,
);


