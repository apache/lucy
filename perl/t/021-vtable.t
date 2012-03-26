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

package MyHash;
use base qw( Lucy::Object::Hash );

sub oodle { }

package RAMFolderOfDeath;
use base qw( Lucy::Store::RAMFolder );

sub open_in {
    my ( $self, $filename ) = @_;
    die "Sweet, sweet death.";
}

package OnceRemoved;
use base qw( Lucy::Object::Obj );

our $serialize_was_called = 0;
sub serialize {
    my ( $self, $outstream ) = @_;
    $serialize_was_called++;
    $self->SUPER::serialize($outstream);
}

package TwiceRemoved;
use base qw( OnceRemoved );

package main;

use Lucy::Test;
use Test::More tests => 9;
use Storable qw( nfreeze );

{
    my $twice_removed = TwiceRemoved->new;
    # This triggers a call to Obj_Serialize() via the VTable dispatch.
    my $frozen = nfreeze($twice_removed);
    ok( $serialize_was_called,
        "Overridden method in intermediate class recognized" );
    my $vtable = $twice_removed->get_vtable;
    is( $vtable->get_name, "TwiceRemoved", "correct class" );
    my $parent_vtable = $vtable->get_parent;
    is( $parent_vtable->get_name, "OnceRemoved", "correct parent class" )
}

my $stringified;
my $storage = Lucy::Object::Hash->new;

{
    my $subclassed_hash = MyHash->new;
    $stringified = $subclassed_hash->to_string;

    isa_ok( $subclassed_hash, "MyHash", "Perl isa reports correct subclass" );

   # Store the subclassed object.  At the end of this block, the Perl object
   # will go out of scope and DESTROY will be called, but the Clownfish object
   # will persist.
    $storage->store( "test", $subclassed_hash );
}

my $resurrected = $storage->_fetch("test");

isa_ok( $resurrected, "MyHash", "subclass name survived Perl destruction" );
is( $resurrected->to_string, $stringified,
    "It's the same Hash from earlier (though a different Perl object)" );

my $booga = Lucy::Object::CharBuf->new("booga");
$resurrected->store( "ooga", $booga );

is( $resurrected->fetch("ooga"),
    "booga", "subclassed object still performs correctly at the C level" );

my $methods = Lucy::Object::VTable->novel_host_methods('MyHash');
is_deeply( $methods->to_perl, ['oodle'], "novel_host_methods" );

my $folder = RAMFolderOfDeath->new;
eval { $folder->slurp_file('foo') };    # calls open_in, which dies per above.
like( $@, qr/sweet/i, "override vtable method with pure perl method" );
