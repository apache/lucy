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

package KinoSearch::Index::SortCache;
use KinoSearch;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = KinoSearch   PACKAGE = KinoSearch::Index::SortCache

SV*
value(self, ...)
    kino_SortCache *self;
CODE:
{
    SV *ord_sv = NULL;
    int32_t ord = 0;

    XSBind_allot_params( &(ST(0)), 1, items, 
        "KinoSearch::Index::SortCache::value_PARAMS",
        &ord_sv, "ord", 3, 
        NULL);
    if (ord_sv) { ord = SvIV(ord_sv); }
    else { THROW(KINO_ERR, "Missing required param 'ord'"); }

    {
        kino_Obj *blank = Kino_SortCache_Make_Blank(self);
        kino_Obj *value = Kino_SortCache_Value(self, ord, blank);
        RETVAL = XSBind_cfish_to_perl(value);
        LUCY_DECREF(blank);
    }
}
OUTPUT: RETVAL
END_XS_CODE

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::SortCache",
    xs_code           => $xs_code,
    bind_constructors => ["new"],
    bind_methods      => [qw( Ordinal Find )],
);


