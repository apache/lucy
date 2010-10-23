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

use Clownfish::Method;
use Clownfish::Parser;

my $parser = Clownfish::Parser->new;
$parser->parcel_definition('parcel Neato;')
    or die "failed to process parcel_definition";

my %args = (
    return_type => $parser->type('Obj*'),
    class_name  => 'Neato::Foo',
    class_cnick => 'Foo',
    param_list  => $parser->param_list('(Foo *self)'),
    macro_sym   => 'Return_An_Obj',
    parcel      => 'Neato',
);

my $orig      = Clownfish::Method->new(%args);
my $overrider = Clownfish::Method->new(
    %args,
    param_list  => $parser->param_list('(FooJr *self)'),
    class_name  => 'Neato::Foo::FooJr',
    class_cnick => 'FooJr'
);
$overrider->override($orig);
ok( !$overrider->novel );

