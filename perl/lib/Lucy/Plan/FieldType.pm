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

package Lucy::Plan::FieldType;
use Lucy;
our $VERSION = '0.003001';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $synopis = <<'END_SYNOPSIS';

    my @sortable;
    for my $field ( @{ $schema->all_fields } ) {
        my $type = $schema->fetch_type($field);
        next unless $type->sortable;
        push @sortable, $field;
    }

END_SYNOPSIS

Clownfish::CFC::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Plan::FieldType",
    bind_methods => [
        qw(
            Get_Boost
            Indexed
            Stored
            Sortable
            Binary
            Compare_Values
            )
    ],
    bind_constructors => ["new|init2"],
    make_pod          => {
        synopsis => $synopis,
        methods  => [
            qw(
                get_boost
                indexed
                stored
                sortable
                binary
                )
        ],
    }
);


