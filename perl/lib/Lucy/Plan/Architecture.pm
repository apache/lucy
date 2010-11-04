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

package Lucy::Plan::Architecture;
use Lucy;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    package MyArchitecture;
    use base qw( Lucy::Plan::Architecture );

    use LucyX::Index::ZlibDocWriter;
    use LucyX::Index::ZlibDocReader;

    sub register_doc_writer {
        my ( $self, $seg_writer ) = @_; 
        my $doc_writer = LucyX::Index::ZlibDocWriter->new(
            snapshot   => $seg_writer->get_snapshot,
            segment    => $seg_writer->get_segment,
            polyreader => $seg_writer->get_polyreader,
        );  
        $seg_writer->register(
            api       => "Lucy::Index::DocReader",
            component => $doc_writer,
        );  
        $seg_writer->add_writer($doc_writer);
    }

    sub register_doc_reader {
        my ( $self, $seg_reader ) = @_; 
        my $doc_reader = LucyX::Index::ZlibDocReader->new(
            schema   => $seg_reader->get_schema,
            folder   => $seg_reader->get_folder,
            segments => $seg_reader->get_segments,
            seg_tick => $seg_reader->get_seg_tick,
            snapshot => $seg_reader->get_snapshot,
        );  
        $seg_reader->register(
            api       => 'Lucy::Index::DocReader',
            component => $doc_reader,
        );  
    }
 
    package MySchema;
    use base qw( Lucy::Plan::Schema );
    
    sub architecture { 
        shift;
        return MyArchitecture->new(@_); 
    }
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $arch = Lucy::Plan::Architecture->new;
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Plan::Architecture",
    bind_methods => [
        qw(
            Index_Interval
            Skip_Interval
            Init_Seg_Reader
            Register_Doc_Writer
            Register_Doc_Reader
            Register_Deletions_Writer
            Register_Deletions_Reader
            Register_Lexicon_Reader
            Register_Posting_List_Writer
            Register_Posting_List_Reader
            Register_Sort_Writer
            Register_Sort_Reader
            Register_Highlight_Writer
            Register_Highlight_Reader
            Make_Similarity
            )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis => $synopsis,
        methods  => [
            qw(
                register_doc_writer
                register_doc_reader
                )
        ],
        constructors => [ { sample => $constructor } ],
    }
);


