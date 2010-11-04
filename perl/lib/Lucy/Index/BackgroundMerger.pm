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

package Lucy::Index::BackgroundMerger;
use Lucy;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $bg_merger = Lucy::Index::BackgroundMerger->new(
        index  => '/path/to/index',
    );
    $bg_merger->commit;
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $bg_merger = Lucy::Index::BackgroundMerger->new(
        index   => '/path/to/index',    # required
        manager => $manager             # default: created internally
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Index::BackgroundMerger",
    bind_methods => [
        qw(
            Commit
            Prepare_Commit
            Optimize
            )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        methods => [
            qw(
                commit
                prepare_commit
                optimize
                )
        ],
        synopsis     => $synopsis,
        constructors => [ { sample => $constructor } ],
    },
);


