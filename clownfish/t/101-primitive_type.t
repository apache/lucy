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

package MyPrimitiveType;
use base qw( Clownfish::Type::Primitive );

package main;
use Test::More tests => 4;

my $type = MyPrimitiveType->new( specifier => 'hump_t' );
ok( $type->is_primitive, "is_primitive" );

my $other = MyPrimitiveType->new( specifier => 'hump_t' );
ok( $type->equals($other), "equals()" );

$other = MyPrimitiveType->new( specifier => 'dump_t' );
ok( !$type->equals($other), "equals() spoiled by specifier" );

$other = MyPrimitiveType->new( specifier => 'hump_t', const => 1 );
ok( !$type->equals($other), "equals() spoiled by const" );

