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
use Exporter;
BEGIN {
    our @ISA       = qw( Exporter );
    our @EXPORT_OK = qw( list );
}

# Return a partial list of Lucy classes which were once public but are
# now either deprecated, removed, or moved.

sub redacted {
    return qw(
        KinoSearch::Analysis::LCNormalizer
        KinoSearch::Analysis::Token
        KinoSearch::Analysis::TokenBatch
        KinoSearch::Index::Term
        KinoSearch::InvIndex
        KinoSearch::InvIndexer
        KinoSearch::QueryParser::QueryParser
        KinoSearch::Search::BooleanQuery
        KinoSearch::Search::QueryFilter
        KinoSearch::Search::SearchServer
        KinoSearch::Search::SearchClient
    );
}

# Hide additional stuff from PAUSE and search.cpan.org.
sub hidden {
    return qw(
        KinoSearch::Analysis::Inversion
        KinoSearch::Object::Num
        KinoSearch::Plan::Int32Type
        KinoSearch::Plan::Int64Type
        KinoSearch::Plan::Float32Type
        KinoSearch::Plan::Float64Type
        Lucy::Redacted
        KinoSearch::Test::Object::TestCharBuf
        Lucy::Test::TestUtils
        Lucy::Test::USConSchema
        KinoSearch::Util::BitVector
    );
}

1;
