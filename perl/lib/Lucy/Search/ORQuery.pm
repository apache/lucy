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

package Lucy::Search::ORQuery;
use Lucy;
our $VERSION = '0.003002';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $foo_or_bar_query = Lucy::Search::ORQuery->new(
        children => [ $foo_query, $bar_query ],
    );
    my $hits = $searcher->hits( query => $foo_or_bar_query );
    ...
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $foo_or_bar_query = Lucy::Search::ORQuery->new(
        children => [ $foo_query, $bar_query ],
    );
END_CONSTRUCTOR

Clownfish::CFC::Binding::Perl::Class->register(
    parcel            => "Lucy",
    class_name        => "Lucy::Search::ORQuery",
    bind_constructors => ["new"],
    make_pod          => {
        methods     => [qw( add_child )],
        synopsis    => $synopsis,
        constructor => { sample => $constructor, }
    },
);


