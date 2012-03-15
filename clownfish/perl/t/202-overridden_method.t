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

use Test::More tests => 1;

use Clownfish::CFC::Model::Method;
use Clownfish::CFC::Parser;

my $parser = Clownfish::CFC::Parser->new;
$parser->parse('parcel Neato;')
    or die "failed to process parcel_definition";

my %args = (
    return_type => $parser->parse('Obj*'),
    class_name  => 'Neato::Foo',
    class_cnick => 'Foo',
    param_list  => $parser->parse('(Foo *self)'),
    macro_sym   => 'Return_An_Obj',
    parcel      => 'Neato',
);

my $orig      = Clownfish::CFC::Model::Method->new(%args);
my $overrider = Clownfish::CFC::Model::Method->new(
    %args,
    param_list  => $parser->parse('(FooJr *self)'),
    class_name  => 'Neato::Foo::FooJr',
    class_cnick => 'FooJr'
);
$overrider->override($orig);
ok( !$overrider->novel, "A Method which overrides another is not 'novel'" );

