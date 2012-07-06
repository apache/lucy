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

package Lucy::Store::LockFactory;
use Lucy;
our $VERSION = '0.003002';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    use Sys::Hostname qw( hostname );
    my $hostname = hostname() or die "Can't get unique hostname";
    my $folder = Lucy::Store::FSFolder->new( 
        path => '/path/to/index', 
    );
    my $lock_factory = Lucy::Store::LockFactory->new(
        folder => $folder,
        host   => $hostname,
    );
    my $write_lock = $lock_factory->make_lock(
        name     => 'write',
        timeout  => 5000,
        interval => 100,
    );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $lock_factory = Lucy::Store::LockFactory->new(
        folder => $folder,      # required
        host   => $hostname,    # required
    );
END_CONSTRUCTOR

Clownfish::CFC::Binding::Perl::Class->register(
    parcel            => "Lucy",
    class_name        => "Lucy::Store::LockFactory",
    bind_methods      => [qw( Make_Lock Make_Shared_Lock )],
    bind_constructors => ["new"],
    make_pod          => {
        methods     => [qw( make_lock make_shared_lock)],
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    }
);


