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

use Test::More tests => 11;
use Storable qw( nfreeze thaw );
use Lucy::Test;

my $doc = Lucy::Document::Doc->new;
is_deeply( $doc->get_fields, {}, "get_fields" );
is( $doc->get_doc_id, 0, "default doc_id of 0" );

$doc->set_fields( { foo => 'oink' } );
is_deeply( $doc->get_fields, { foo => 'oink' }, "set_fields" );

$doc->{foo} = "blah";
is_deeply( $doc->get_fields, { foo => 'blah' }, "overloading" );

my %hash = ( foo => 'foo' );
$doc = Lucy::Document::Doc->new(
    fields => \%hash,
    doc_id => 30,
);
$hash{bar} = "blah";
is_deeply(
    $doc->get_fields,
    { foo => 'foo', bar => 'blah' },
    "using supplied hash"
);
is( $doc->get_doc_id, 30, "doc_id param" );
$doc->set_doc_id(20);
is( $doc->get_doc_id, 20, "doc_id param" );

my $frozen = nfreeze($doc);
my $thawed = thaw($frozen);
is_deeply( $thawed->get_fields, $doc->get_fields,
    "fields survive freeze/thaw" );
is( $thawed->get_doc_id, $doc->get_doc_id, "doc_id survives freeze/thaw" );
ok( $doc->equals($thawed), "equals" );

my $dump   = $doc->dump;
my $loaded = $doc->load($dump);
ok( $doc->equals($loaded), "dump => load round trip" );
