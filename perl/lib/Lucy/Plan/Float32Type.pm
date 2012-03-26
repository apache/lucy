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

package Lucy::Plan::Float32Type;
use Lucy;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $schema       = Lucy::Plan::Schema->new;
    my $float32_type = Lucy::Plan::FloatType->new;
    $schema->spec_field( name => 'intensity', type => $float32_type );
END_SYNOPSIS
my $constructor = <<'END_CONSTRUCTOR';
    my $float32_type = Lucy::Plan::Float32Type->new(
        indexed  => 0,    # default true
        stored   => 0,    # default true
        sortable => 1,    # default false
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "Lucy",
    class_name        => "Lucy::Plan::Float32Type",
    bind_constructors => ["new|init2"],
    #make_pod          => {
    #    synopsis    => $synopsis,
    #    constructor => { sample => $constructor },
    #},
);


