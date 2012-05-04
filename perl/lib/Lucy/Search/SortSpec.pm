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

package Lucy::Search::SortSpec;
use Lucy;
our $VERSION = '0.003001';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $sort_spec = Lucy::Search::SortSpec->new(
        rules => [
            Lucy::Search::SortRule->new( field => 'date' ),
            Lucy::Search::SortRule->new( type  => 'doc_id' ),
        ],
    );
    my $hits = $searcher->hits(
        query     => $query,
        sort_spec => $sort_spec,
    );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $sort_spec = Lucy::Search::SortSpec->new( rules => \@rules );
END_CONSTRUCTOR

Clownfish::CFC::Binding::Perl::Class->register(
    parcel            => "Lucy",
    class_name        => "Lucy::Search::SortSpec",
    bind_methods      => [qw( Get_Rules )],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    },
);


