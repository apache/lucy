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
use Storable qw( nfreeze thaw );
use Lucy::Test;

my $doc = Lucy::Document::HitDoc->new;
is( $doc->get_doc_id, 0,   "default doc_id of 0" );
is( $doc->get_score,  0.0, "default score of 0.0" );
$doc->set_score(2);
is( $doc->get_score, 2, "set_score" );

$doc->{foo} = "foo foo";
is( $doc->{foo}, "foo foo", "hash overloading" );

my $frozen = nfreeze($doc);
my $thawed = thaw($frozen);
is( ref($thawed),       ref($doc),       "correct class after freeze/thaw" );
is( $thawed->get_score, $doc->get_score, "score survives freeze/thaw" );
ok( $doc->equals($thawed), "equals" );

my $dump   = $doc->dump;
my $loaded = $doc->load($dump);
ok( $doc->equals($loaded), "dump => load round trip" );
