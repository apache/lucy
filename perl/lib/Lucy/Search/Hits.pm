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

package Lucy::Search::Hits;
use Lucy;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $hits = $searcher->hits(
        query      => $query,
        offset     => 0,
        num_wanted => 10,
    );
    while ( my $hit = $hits->next ) {
        print "<p>$hit->{title} <em>" . $hit->get_score . "</em></p>\n";
    }
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Search::Hits",
    bind_methods => [
        qw(
            Total_Hits
            Next
            )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis => $synopsis,
        methods  => [qw( next total_hits )],
    }
);


