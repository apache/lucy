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
use lib 'buildlib';

use Test::More tests => 4;
use Lucy::Test;

my $folder = Lucy::Store::RAMFolder->new;
my $schema = Lucy::Test::TestSchema->new;

my $indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
);

$indexer->add_doc( { content => 'foo' } );
undef $indexer;

$indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
);
$indexer->add_doc( { content => 'foo' } );
pass("Indexer ignores garbage from interrupted session");

SKIP: {
    skip( "Known leak, though might be fixable", 2 ) if $ENV{LUCY_VALGRIND};
    eval {
        my $manager
            = Lucy::Index::IndexManager->new( host => 'somebody_else' );
        my $inv = Lucy::Index::Indexer->new(
            manager => $manager,
            index   => $folder,
            schema  => $schema,
        );
    };
    like( $@, qr/write.lock/, "failed to get lock with competing host" );
    isa_ok( $@, "Lucy::Store::LockErr", "Indexer throws a LockErr" );
}

my $pid = 12345678;
do {
    # Fake a write lock.
    $folder->delete("locks/write.lock") or die "Couldn't delete 'write.lock'";
    my $outstream = $folder->open_out('locks/write.lock')
        or die Clownfish->error;
    while ( kill( 0, $pid ) ) {
        $pid++;
    }
    $outstream->print(
        qq|
        {  
            "host": "somebody_else",
            "pid": $pid,
            "name": "write"
        }|
    );
    $outstream->close;

    eval {
        my $manager
            = Lucy::Index::IndexManager->new( host => 'somebody_else' );
        my $inv = Lucy::Index::Indexer->new(
            manager => $manager,
            schema  => $schema,
            index   => $folder,
        );
    };

    # Mitigate (though not eliminate) race condition false failure.
} while ( kill( 0, $pid ) );

ok( !$@, "clobbered lock from same host with inactive pid" );
