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

package Lucy::Index::PostingList;
use Lucy;
our $VERSION = '0.003002';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $posting_list_reader 
        = $seg_reader->obtain("Lucy::Index::PostingListReader");
    my $posting_list = $posting_list_reader->posting_list( 
        field => 'content',
        term  => 'foo',
    );
    while ( my $doc_id = $posting_list->next ) {
        say "Matching doc id: $doc_id";
    }
END_SYNOPSIS

Clownfish::CFC::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Index::PostingList",
    bind_methods => [
        qw(
            Seek
            Get_Posting
            Get_Doc_Freq
            Make_Matcher
            )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis => $synopsis,
        methods  => [
            qw(
                next
                advance
                get_doc_id
                get_doc_freq
                seek
                )
        ],
    },
);


