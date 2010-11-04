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

package Lucy::Index::DeletionsWriter;
use Lucy;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $polyreader  = $del_writer->get_polyreader;
    my $seg_readers = $polyreader->seg_readers;
    for my $seg_reader (@$seg_readers) {
        my $count = $del_writer->seg_del_count( $seg_reader->get_seg_name );
        ...
    }
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Index::DeletionsWriter",
    bind_methods => [
        qw(
            Generate_Doc_Map
            Delete_By_Term
            Delete_By_Query
            Delete_By_Doc_ID
            Updated
            Seg_Deletions
            Seg_Del_Count
            )
    ],
    make_pod => {
        synopsis => $synopsis,
        methods  => [
            qw(
                delete_by_term
                delete_by_query
                updated
                seg_del_count
                )
        ],
    },
);
Clownfish::Binding::Perl::Class->register(
    parcel            => "Lucy",
    class_name        => "Lucy::Index::DefaultDeletionsWriter",
    bind_constructors => ["new"],
);


