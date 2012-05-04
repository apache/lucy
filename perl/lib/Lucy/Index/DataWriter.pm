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

package Lucy::Index::DataWriter;
use Lucy;
our $VERSION = '0.003001';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $synopsis = <<END_SYNOPSIS;
    # Abstract base class.
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $writer = MyDataWriter->new(
        snapshot   => $snapshot,      # required
        segment    => $segment,       # required
        polyreader => $polyreader,    # required
    );
END_CONSTRUCTOR

Clownfish::CFC::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Index::DataWriter",
    bind_methods => [
        qw(
            Add_Inverted_Doc
            Add_Segment
            Delete_Segment
            Merge_Segment
            Finish
            Format
            Metadata
            Get_Snapshot
            Get_Segment
            Get_PolyReader
            Get_Schema
            Get_Folder
            )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
        methods     => [
            qw(
                add_inverted_doc
                add_segment
                delete_segment
                merge_segment
                finish
                format
                metadata
                get_snapshot
                get_segment
                get_polyreader
                get_schema
                get_folder
                )
        ],
    },
);


