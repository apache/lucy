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

package Lucy::Search::Searcher;
use Lucy;

1;

__END__

__BINDING__

my $constructor = <<'END_CONSTRUCTOR';
    package MySearcher;
    use base qw( Lucy::Search::Searcher );
    sub new {
        my $self = shift->SUPER::new;
        ...
        return $self;
    }
END_CONSTRUCTOR

Clownfish::CFC::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Search::Searcher",
    bind_methods => [
        qw( Doc_Max
            Doc_Freq
            Glean_Query
            Hits
            Collect
            Top_Docs
            Fetch_Doc
            Fetch_Doc_Vec
            Get_Schema
            Close )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => "    # Abstract base class.\n",
        constructor => { sample => $constructor },
        methods     => [
            qw(
                hits
                collect
                glean_query
                doc_max
                doc_freq
                fetch_doc
                get_schema
                )
        ],
    },
);


