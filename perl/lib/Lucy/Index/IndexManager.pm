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

package Lucy::Index::IndexManager;
use Lucy;
our $VERSION = '0.003001';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    use Sys::Hostname qw( hostname );
    my $hostname = hostname() or die "Can't get unique hostname";
    my $manager = Lucy::Index::IndexManager->new( 
        host => $hostname,
    );

    # Index time:
    my $indexer = Lucy::Index::Indexer->new(
        index => '/path/to/index',
        manager => $manager,
    );

    # Search time:
    my $reader = Lucy::Index::IndexReader->open(
        index   => '/path/to/index',
        manager => $manager,
    );
    my $searcher = Lucy::Search::IndexSearcher->new( index => $reader );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $manager = Lucy::Index::IndexManager->new(
        host => $hostname,    # default: ""
    );
END_CONSTRUCTOR

Clownfish::CFC::Binding::Perl::Class->register(
    parcel            => "Lucy",
    class_name        => "Lucy::Index::IndexManager",
    bind_constructors => ["new"],
    bind_methods      => [
        qw(
            Recycle
            Make_Write_Lock
            Make_Deletion_Lock
            Make_Merge_Lock
            Make_Snapshot_Read_Lock
            Highest_Seg_Num
            Make_Snapshot_Filename
            Set_Folder
            Get_Folder
            Get_Host
            Set_Write_Lock_Timeout
            Get_Write_Lock_Timeout
            Set_Write_Lock_Interval
            Get_Write_Lock_Interval
            Set_Merge_Lock_Timeout
            Get_Merge_Lock_Timeout
            Set_Merge_Lock_Interval
            Get_Merge_Lock_Interval
            Set_Deletion_Lock_Timeout
            Get_Deletion_Lock_Timeout
            Set_Deletion_Lock_Interval
            Get_Deletion_Lock_Interval
            )
    ],
    make_pod => {
        methods => [
            qw(
                make_write_lock
                recycle
                set_folder
                get_folder
                get_host
                set_write_lock_timeout
                get_write_lock_timeout
                set_write_lock_interval
                get_write_lock_interval
                )
        ],
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    },
);


