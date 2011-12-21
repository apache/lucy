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

use strict;
use warnings;

use Test::More tests => 10;
use Clownfish::CFC::Type;
use Clownfish::CFC::Parser;

my $foo_type = Clownfish::CFC::Type->new_arbitrary(
    parcel    => 'Neato',
    specifier => "foo_t",
);
is( $foo_type->get_specifier, "foo_t", "get_specifier" );
is( $foo_type->to_c,          "foo_t", "to_c" );

my $compare_t_type = Clownfish::CFC::Type->new_arbitrary(
    parcel    => 'Neato',
    specifier => "Sort_compare_t",
);
is( $compare_t_type->get_specifier,
    "neato_Sort_compare_t", "Prepend prefix to specifier" );
is( $compare_t_type->to_c, "neato_Sort_compare_t", "to_c" );

my $twin = Clownfish::CFC::Type->new_arbitrary(
    parcel    => 'Neato',
    specifier => "foo_t",
);
ok( $foo_type->equals($twin), "equals" );
ok( !$foo_type->equals($compare_t_type),
    "equals spoiled by different specifier"
);

my $parser = Clownfish::CFC::Parser->new;

for my $specifier (qw( foo_t Sort_compare_t )) {
    my $type = $parser->parse($specifier);
    ok( $type && $type->is_arbitrary, "arbitrary_type '$specifier'" );
SKIP: {
        skip( "Can't recover from bad specifier", 1 );
        ok( !$parser->parse( $specifier . "_y_p_e eep;" ),
            "arbitrary_type_specifier guards against partial word matches" );
    }
}
