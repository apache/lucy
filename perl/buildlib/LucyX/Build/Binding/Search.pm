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
package LucyX::Build::Binding::Search;
use strict;
use warnings;

our $VERSION = '0.004002';
$VERSION = eval $VERSION;

sub bind_all {
    my $class = shift;
    $class->bind_filter;
    $class->bind_mockmatcher;
    $class->bind_proximityquery;
    $class->bind_proximitycompiler;
}

sub bind_filter {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "LucyX::Search::FilterMatcher",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_mockmatcher {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "LucyX::Search::MockMatcher",
    );
    $binding->bind_constructor( alias => '_new' );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_proximityquery {
    my @exposed = qw( Get_Field Get_Terms Get_Within );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $proximity_query = LucyX::Search::ProximityQuery->new( 
        field  => 'content',
        terms  => [qw( the who )],
        within => 10,    # match within 10 positions
    );
    my $hits = $searcher->hits( query => $proximity_query );
END_SYNOPSIS
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new' );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "LucyX::Search::ProximityQuery",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_proximitycompiler {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "LucyX::Search::ProximityCompiler",
    );
    $binding->bind_constructor( alias => 'do_new' );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

1;
