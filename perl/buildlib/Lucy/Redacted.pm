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

package Lucy::Redacted;

our $VERSION = '0.004002';
$VERSION = eval $VERSION;

use Exporter 'import';
our @EXPORT_OK = qw( list );

# Return a partial list of Lucy classes which were once public but are
# now either deprecated, removed, or moved.

sub redacted {
    return qw(
        Lucy::Analysis::LCNormalizer
        Lucy::Analysis::Token
        Lucy::Analysis::TokenBatch
        Lucy::Index::Term
        Lucy::InvIndex
        Lucy::InvIndexer
        Lucy::Object::Obj
        Lucy::QueryParser::QueryParser
        Lucy::Search::BooleanQuery
        Lucy::Search::QueryFilter
        Lucy::Search::SearchServer
        Lucy::Search::SearchClient
    );
}

# Hide additional stuff from PAUSE and search.cpan.org.
sub hidden {
    return qw(
        Lucy::Analysis::Inversion
        Clownfish::Num
        Lucy::Plan::Int32Type
        Lucy::Plan::Int64Type
        Lucy::Plan::Float32Type
        Lucy::Plan::Float64Type
        Lucy::Redacted
        Lucy::Test::Object::TestCharBuf
        Lucy::Test::TestUtils
        Lucy::Util::BitVector
    );
}

1;
