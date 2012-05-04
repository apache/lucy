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

package Lucy::Index::SegReader;
use Lucy;
our $VERSION = '0.003001';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $polyreader = Lucy::Index::IndexReader->open(
        index => '/path/to/index',
    );
    my $seg_readers = $polyreader->seg_readers;
    for my $seg_reader (@$seg_readers) {
        my $seg_name = $seg_reader->get_seg_name;
        my $num_docs = $seg_reader->doc_max;
        print "Segment $seg_name ($num_docs documents):\n";
        my $doc_reader = $seg_reader->obtain("Lucy::Index::DocReader");
        for my $doc_id ( 1 .. $num_docs ) {
            my $doc = $doc_reader->fetch_doc($doc_id);
            print "  $doc_id: $doc->{title}\n";
        }
    }
END_SYNOPSIS

Clownfish::CFC::Binding::Perl::Class->register(
    parcel            => "Lucy",
    class_name        => "Lucy::Index::SegReader",
    bind_methods      => [qw( Get_Seg_Name Get_Seg_Num Register )],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis => $synopsis,
        methods  => [qw( Get_Seg_Name Get_Seg_Num )],
    },
);


