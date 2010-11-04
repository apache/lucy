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

package Lucy::Index::PolyReader;
use Lucy;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $polyreader = Lucy::Index::IndexReader->open( 
        index => '/path/to/index',
    );
    my $doc_reader = $polyreader->obtain("Lucy::Index::DocReader");
    for my $doc_id ( 1 .. $polyreader->doc_max ) {
        my $doc = $doc_reader->fetch_doc($doc_id);
        print " $doc_id: $doc->{title}\n";
    }
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel            => "Lucy",
    class_name        => "Lucy::Index::PolyReader",
    bind_constructors => [ 'new', 'open|do_open' ],
    bind_methods      => [qw( Get_Seg_Readers )],
    make_pod          => { synopsis => $synopsis },
);


