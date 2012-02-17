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
package Lucy::Build::Binding::Search::Collector;
use strict;
use warnings;

sub bind_all {
    my $class = shift;
    $class->bind_bitcollector;
    $class->bind_sortcollector;
}

sub bind_bitcollector {
    my @exposed = qw( Collect );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $bit_vec = Lucy::Object::BitVector->new(
        capacity => $searcher->doc_max + 1,
    );
    my $bit_collector = Lucy::Search::Collector::BitCollector->new(
        bit_vector => $bit_vec, 
    );
    $searcher->collect(
        collector => $bit_collector,
        query     => $query,
    );
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $bit_collector = Lucy::Search::Collector::BitCollector->new(
        bit_vector => $bit_vec,    # required
    );
END_CONSTRUCTOR
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::Collector::BitCollector",
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_sortcollector {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::Collector::SortCollector",
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

1;
