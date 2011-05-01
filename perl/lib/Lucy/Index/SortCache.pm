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

package Lucy::Index::SortCache;
use Lucy;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Index::SortCache

SV*
value(self, ...)
    lucy_SortCache *self;
CODE:
{
    int32_t ord = 0;
    chy_bool_t args_ok
        = XSBind_allot_params(&(ST(0)), 1, items,
                              "Lucy::Index::SortCache::value_PARAMS",
                              ALLOT_I32(&ord, "ord", 3, false),
                              NULL);
    if (!args_ok) {
        CFISH_RETHROW(LUCY_INCREF(cfish_Err_get_error()));
    }
    {
        lucy_Obj *blank = Lucy_SortCache_Make_Blank(self);
        lucy_Obj *value = Lucy_SortCache_Value(self, ord, blank);
        RETVAL = XSBind_cfish_to_perl(value);
        LUCY_DECREF(blank);
    }
}
OUTPUT: RETVAL
END_XS_CODE

Clownfish::Binding::Perl::Class->register(
    parcel            => "Lucy",
    class_name        => "Lucy::Index::SortCache",
    xs_code           => $xs_code,
    bind_methods      => [qw( Ordinal Find )],
);


