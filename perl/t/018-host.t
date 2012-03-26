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

use Test::More tests => 8;
use Lucy::Test;
use Lucy qw( to_perl to_clownfish );

my $object = Lucy::Object::Host->new();
isa_ok( $object, "Lucy::Object::Host" );

is( $object->_callback,     undef, "void callback" );
is( $object->_callback_f64, 5,     "f64 callback" );
is( $object->_callback_i64, 5,     "integer callback" );

my $test_obj = $object->_callback_obj;
isa_ok( $test_obj, "Lucy::Object::ByteBuf" );

my %complex_data_structure = (
    a => [ 1, 2, 3, { ooga => 'booga' } ],
    b => { foo => 'foofoo', bar => 'barbar' },
);
my $kobj = to_clownfish( \%complex_data_structure );
isa_ok( $kobj, 'Lucy::Object::Obj' );
my $transformed = to_perl($kobj);
is_deeply( $transformed, \%complex_data_structure,
    "transform from Perl to Clownfish data structures and back" );

my $bread_and_butter = Lucy::Object::Hash->new;
$bread_and_butter->store( 'bread', Lucy::Object::ByteBuf->new('butter') );
my $salt_and_pepper = Lucy::Object::Hash->new;
$salt_and_pepper->store( 'salt', Lucy::Object::ByteBuf->new('pepper') );
$complex_data_structure{c} = $bread_and_butter;
$complex_data_structure{d} = $salt_and_pepper;
$transformed = to_perl( to_clownfish( \%complex_data_structure ) );
$complex_data_structure{c} = { bread => 'butter' };
$complex_data_structure{d} = { salt  => 'pepper' };
is_deeply( $transformed, \%complex_data_structure,
    "handle mixed data structure correctly" );
