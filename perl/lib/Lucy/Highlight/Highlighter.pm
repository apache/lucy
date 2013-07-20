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

package Lucy::Highlight::Highlighter;
use Lucy;
our $VERSION = '0.003003';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $highlighter = Lucy::Highlight::Highlighter->new(
        searcher => $searcher,
        query    => $query,
        field    => 'body'
    );
    my $hits = $searcher->hits( query => $query );
    while ( my $hit = $hits->next ) {
        my $excerpt = $highlighter->create_excerpt($hit);
        ...
    }
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $highlighter = Lucy::Highlight::Highlighter->new(
        searcher       => $searcher,    # required
        query          => $query,       # required
        field          => 'content',    # required
        excerpt_length => 150,          # default: 200
    );
END_CONSTRUCTOR

Clownfish::CFC::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Highlight::Highlighter",
    bind_methods => [
        qw(
            Highlight
            Encode
            Create_Excerpt
            _find_best_fragment|Find_Best_Fragment
            _raw_excerpt|Raw_Excerpt
            _highlight_excerpt|Highlight_Excerpt
            Find_Sentences
            Set_Pre_Tag
            Get_Pre_Tag
            Set_Post_Tag
            Get_Post_Tag
            Get_Searcher
            Get_Query
            Get_Compiler
            Get_Excerpt_Length
            Get_Field
            )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
        methods     => [
            qw(
                create_excerpt
                highlight
                encode
                set_pre_tag
                get_pre_tag
                set_post_tag
                get_post_tag
                get_searcher
                get_query
                get_compiler
                get_excerpt_length
                get_field
                )
        ]
    },
);


