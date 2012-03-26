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

use Test::More tests => 20;

package TestObj;
use base qw( Lucy::Object::Obj );

our $version = $Lucy::VERSION;

package SonOfTestObj;
use base qw( TestObj );
{
    sub to_string {
        my $self = shift;
        return "STRING: " . $self->SUPER::to_string;
    }

    sub serialize {
        my ( $self, $outstream ) = @_;
        $self->SUPER::serialize($outstream);
        $outstream->write_string("zowie");
    }

    sub deserialize {
        my ( $self, $instream ) = @_;
        $self = $self->SUPER::deserialize($instream);
        $instream->read_string;
        return $self;
    }
}

package BadSerialize;
use base qw( Lucy::Object::Obj );
{
    sub serialize { }
}

package BadDump;
use base qw( Lucy::Object::Obj );
{
    sub dump { }
}

package main;
use Storable qw( freeze thaw );

ok( defined $TestObj::version,
    "Using base class should grant access to "
        . "package globals in the Lucy:: namespace"
);

# TODO: Port this test to C.
eval { my $foo = Lucy::Object::Obj->new };
like( $@, qr/abstract/i, "Obj is an abstract class" );

my $object = TestObj->new;
isa_ok( $object, "Lucy::Object::Obj",
    "Clownfish objects can be subclassed outside the Lucy hierarchy" );

# TODO: Port this test to C.
eval { my $twin = $object->clone };
like( $@, qr/abstract/i, "clone throws an abstract method exception" );

ok( $object->is_a("Lucy::Object::Obj"), "custom is_a correct" );
ok( !$object->is_a("Lucy::Object"),     "custom is_a too long" );
ok( !$object->is_a("Lucy"),             "custom is_a substring" );
ok( !$object->is_a(""),                 "custom is_a blank" );
ok( !$object->is_a("thing"),            "custom is_a wrong" );

eval { my $another_obj = TestObj->new( kill_me_now => 1 ) };
like( $@, qr/kill_me_now/, "reject bad param" );

my $stringified_perl_obj = "$object";
require Lucy::Object::Hash;
my $hash = Lucy::Object::Hash->new;
$hash->store( foo => $object );
is( $object->get_refcount, 2, "refcount increased via C code" );
is( $object->get_refcount, 2, "refcount increased via C code" );
undef $object;
$object = $hash->fetch("foo");
is( "$object", $stringified_perl_obj, "same perl object as before" );

is( $object->get_refcount, 2, "correct refcount after retrieval" );
undef $hash;
is( $object->get_refcount, 1, "correct refcount after destruction of ref" );

my $copy = thaw( freeze($object) );
is( ref($copy), ref($object), "freeze/thaw" );

$object = SonOfTestObj->new;
like( $object->to_string, qr/STRING:.*?SonOfTestObj/,
    "overridden XS bindings can be called via SUPER" );

my $frozen = freeze($object);
my $dupe   = thaw($frozen);
is( ref($dupe), ref($object), "override serialize/deserialize" );

SKIP: {
    skip( "Invalid serialization causes leaks", 1 ) if $ENV{LUCY_VALGRIND};
    my $bad = BadSerialize->new;
    eval { my $froze = freeze($bad); };
    like( $@, qr/empty/i,
        "Don't allow subclasses to perform invalid serialization" );
}

SKIP: {
    skip( "Exception thrown within callback leaks", 1 )
        if $ENV{LUCY_VALGRIND};
    $hash = Lucy::Object::Hash->new;
    $hash->store( foo => BadDump->new );
    eval { $hash->dump };
    like( $@, qr/NULL/,
        "Don't allow methods without nullable return values to return NULL" );
}
