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

package Lucy::Search::QueryParser;
use Lucy;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $query_parser = Lucy::Search::QueryParser->new(
        schema => $searcher->get_schema,
        fields => ['body'],
    );
    my $query = $query_parser->parse( $query_string );
    my $hits  = $searcher->hits( query => $query );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $query_parser = Lucy::Search::QueryParser->new(
        schema         => $searcher->get_schema,    # required
        analyzer       => $analyzer,                # overrides schema
        fields         => ['bodytext'],             # default: indexed fields
        default_boolop => 'AND',                    # default: 'OR'
    );
END_CONSTRUCTOR

Clownfish::CFC::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Search::QueryParser",
    bind_methods => [
        qw(
            Parse
            Tree
            Expand
            Expand_Leaf
            Prune
            Heed_Colons
            Set_Heed_Colons
            Get_Analyzer
            Get_Schema
            Get_Fields
            Make_Term_Query
            Make_Phrase_Query
            Make_AND_Query
            Make_OR_Query
            Make_NOT_Query
            Make_Req_Opt_Query
            )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        methods => [
            qw( parse
                tree
                expand
                expand_leaf
                prune
                set_heed_colons
                make_term_query
                make_phrase_query
                make_and_query
                make_or_query
                make_not_query
                make_req_opt_query
                )
        ],
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    }
);


