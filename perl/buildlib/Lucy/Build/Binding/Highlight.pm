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
package Lucy::Build::Binding::Highlight;
use strict;
use warnings;

our $VERSION = '0.004002';
$VERSION = eval $VERSION;

sub bind_all {
    my $class = shift;
    $class->bind_heatmap;
    $class->bind_highlighter;
}

sub bind_heatmap {
    my $pod_spec    = Clownfish::CFC::Binding::Perl::Pod->new;
    my $constructor = <<'END_CONSTRUCTOR';
    my $heat_map = Lucy::Highlight::HeatMap->new(
        spans  => \@highlight_spans,
        window => 100,
    );
END_CONSTRUCTOR
    $pod_spec->add_constructor( alias => 'new', sample => $constructor );

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Highlight::HeatMap",
    );
    #$binding->set_pod_spec($pod_spec); TODO

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_highlighter {
    my @exposed = qw(
        Create_Excerpt
        Highlight
        Encode
        Set_Pre_Tag
        Get_Pre_Tag
        Set_Post_Tag
        Get_Post_Tag
        Get_Searcher
        Get_Query
        Get_Compiler
        Get_Excerpt_Length
        Get_Field
    );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Highlight::Highlighter",
    );
    $binding->bind_method(
        alias  => '_raw_excerpt',
        method => 'Raw_Excerpt'
    );
    $binding->bind_method(
        alias  => '_highlight_excerpt',
        method => 'Highlight_Excerpt'
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

1;
