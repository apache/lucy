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

package Lucy::Index::DataReader;
use Lucy;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    # Abstract base class.
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $reader = MyDataReader->new(
        schema   => $seg_reader->get_schema,      # default undef
        folder   => $seg_reader->get_folder,      # default undef
        snapshot => $seg_reader->get_snapshot,    # default undef
        segments => $seg_reader->get_segments,    # default undef
        seg_tick => $seg_reader->get_seg_tick,    # default -1
    );
END_CONSTRUCTOR

Clownfish::CFC::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Index::DataReader",
    bind_methods => [
        qw(
            Get_Schema
            Get_Folder
            Get_Segments
            Get_Snapshot
            Get_Seg_Tick
            Get_Segment
            Aggregator
            Close
            )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor, },
        methods     => [
            qw(
                get_schema
                get_folder
                get_snapshot
                get_segments
                get_segment
                get_seg_tick
                aggregator
                )
        ]
    },
);


