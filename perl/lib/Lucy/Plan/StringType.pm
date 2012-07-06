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

package Lucy::Plan::StringType;
use Lucy;
our $VERSION = '0.003002';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $type   = Lucy::Plan::StringType->new;
    my $schema = Lucy::Plan::Schema->new;
    $schema->spec_field( name => 'category', type => $type );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $type = Lucy::Plan::StringType->new(
        boost    => 0.1,    # default: 1.0
        indexed  => 1,      # default: true
        stored   => 1,      # default: true
        sortable => 1,      # default: false
    );
END_CONSTRUCTOR

Clownfish::CFC::Binding::Perl::Class->register(
    parcel            => "Lucy",
    class_name        => "Lucy::Plan::StringType",
    bind_constructors => ["new|init2"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    },
);


