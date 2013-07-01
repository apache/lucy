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

use Test::More tests => 18;
use File::Spec::Functions qw( catfile );

BEGIN { use_ok('Clownfish::CFC::Model::Parcel') }

my $foo = Clownfish::CFC::Model::Parcel->new( name => "Foo" );
isa_ok( $foo, "Clownfish::CFC::Model::Parcel", "new" );
ok( !$foo->included, "not included" );
$foo->register;

my $same_name = Clownfish::CFC::Model::Parcel->new( name => "Foo" );
eval { $same_name->register; };
like( $@, qr/parcel .* already registered/i,
      "can't register two parcels with the same name" );

my $same_nick = Clownfish::CFC::Model::Parcel->new(
    name  => "OtherFoo",
    cnick => "Foo",
);
eval { $same_nick->register; };
like( $@, qr/parcel with nickname .* already registered/i,
      "can't register two parcels with the same nickname" );

my $included_foo = Clownfish::CFC::Model::Parcel->new(
    name        => "IncludedFoo",
    is_included => 1,
);
ok( $included_foo->included, "included" );
$included_foo->register;

my $parcels = Clownfish::CFC::Model::Parcel->all_parcels;
my @names = sort(map { $_->get_name } @$parcels);
is_deeply( \@names, [ "Foo", "IncludedFoo" ], "all_parcels" );

my $dependent_foo = Clownfish::CFC::Model::Parcel->new(
    name        => "DependentFoo",
    is_included => 1,
);
$dependent_foo->register;

$foo->add_inherited_parcel($included_foo);
$foo->add_dependent_parcel($dependent_foo);
my @dep_names = sort(map { $_->get_name } @{ $foo->dependent_parcels });
is_deeply( \@dep_names, [ "DependentFoo", "IncludedFoo" ],
           "dependent_parcels" );
my @inh_names = sort(map { $_->get_name } @{ $foo->inherited_parcels });
is_deeply( \@inh_names, [ "IncludedFoo" ], "inherited_parcels" );

my $json = qq|
        {
            "name": "Crustacean",
            "nickname": "Crust",
            "version": "v0.1.0"
        }
|;
isa_ok(
    Clownfish::CFC::Model::Parcel->new_from_json( json => $json ),
    "Clownfish::CFC::Model::Parcel",
    "new_from_json"
);

isa_ok(
    Clownfish::CFC::Model::Parcel->new_from_file(
        path => catfile(qw( t cfbase Animal.cfp )),
    ),
    "Clownfish::CFC::Model::Parcel",
    "new_from_file"
);

# Register singleton.
my $parcel = Clownfish::CFC::Model::Parcel->new(
    name  => 'Crustacean',
    cnick => 'Crust',
);
$parcel->register;
is( $parcel->get_version->get_vstring, 'v0', "get_version" );

my $thing = Clownfish::CFC::Model::Symbol->new(
    micro_sym => 'sym',
    exposure  => 'parcel',
);
is( $thing->get_prefix, '', 'get_prefix with no parcel' );
is( $thing->get_Prefix, '', 'get_Prefix with no parcel' );
is( $thing->get_PREFIX, '', 'get_PREFIx with no parcel' );

$thing = Clownfish::CFC::Model::Symbol->new(
    micro_sym => 'sym',
    parcel    => 'Crustacean',
    exposure  => 'parcel'
);
is( $thing->get_prefix, 'crust_', 'get_prefix with parcel' );
is( $thing->get_Prefix, 'Crust_', 'get_Prefix with parcel' );
is( $thing->get_PREFIX, 'CRUST_', 'get_PREFIx with parcel' );

