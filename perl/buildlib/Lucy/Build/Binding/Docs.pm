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
package Lucy::Build::Binding::Docs;
use strict;
use warnings;

our $VERSION = '0.004002';
$VERSION = eval $VERSION;

sub bind_all {
    my $class = shift;
    $class->bind_devguide;
    $class->bind_filelocking;
}

sub bind_devguide {
    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $binding  = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Docs::DevGuide",
    );
    $binding->set_pod_spec($pod_spec);
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_filelocking {
    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    use Sys::Hostname qw( hostname );
    my $hostname = hostname() or die "Can't get unique hostname";
    my $manager = Lucy::Index::IndexManager->new( host => $hostname );

    # Index time:
    my $indexer = Lucy::Index::Indexer->new(
        index   => '/path/to/index',
        manager => $manager,
    );

    # Search time:
    my $reader = Lucy::Index::IndexReader->open(
        index   => '/path/to/index',
        manager => $manager,
    );
    my $searcher = Lucy::Search::IndexSearcher->new( index => $reader );
END_SYNOPSIS
    $pod_spec->set_synopsis($synopsis);

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Docs::FileLocking",
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

1;
