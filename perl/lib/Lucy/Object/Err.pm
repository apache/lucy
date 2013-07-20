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

package Lucy::Object::Err;
use Lucy;
our $VERSION = '0.003003';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    use Scalar::Util qw( blessed );
    my $bg_merger;
    while (1) {
        $bg_merger = eval {
            Lucy::Index::BackgroundMerger->new( index => $index );
        };
        last if $bg_merger;
        if ( blessed($@) and $@->isa("Lucy::Store::LockErr") ) {
            warn "Retrying...\n";
        }
        else {
            # Re-throw.
            die "Failed to open BackgroundMerger: $@";
        }
    }
END_SYNOPSIS

Clownfish::CFC::Binding::Perl::Class->register(
    parcel            => "Lucy",
    class_name        => "Lucy::Object::Err",
    bind_methods      => [qw( Cat_Mess Get_Mess )],
    make_pod          => { synopsis => $synopsis },
    bind_constructors => ["_new"],
);


