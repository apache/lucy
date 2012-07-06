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

package Lucy::Search::LeafQuery;
use Lucy;
our $VERSION = '0.003002';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    package MyQueryParser;
    use base qw( Lucy::Search::QueryParser );

    sub expand_leaf {
        my ( $self, $leaf_query ) = @_;
        if ( $leaf_query->get_text =~ /.\*\s*$/ ) {
            return PrefixQuery->new(
                query_string => $leaf_query->get_text,
                field        => $leaf_query->get_field,
            );
        }
        else {
            return $self->SUPER::expand_leaf($leaf_query);
        }
    }
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $leaf_query = Lucy::Search::LeafQuery->new(
        text  => '"three blind mice"',    # required
        field => 'content',               # default: undef
    );
END_CONSTRUCTOR

Clownfish::CFC::Binding::Perl::Class->register(
    parcel            => "Lucy",
    class_name        => "Lucy::Search::LeafQuery",
    bind_methods      => [qw( Get_Field Get_Text )],
    bind_constructors => ["new"],
    make_pod          => {
        methods     => [qw( get_field get_text )],
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    }
);


