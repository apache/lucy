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
package Lucy::Build::Binding::Plan;

sub bind_all {
    my $class = shift;
    $class->bind_architecture;
    $class->bind_blobtype;
    $class->bind_fieldtype;
    $class->bind_float32type;
    $class->bind_float64type;
    $class->bind_fulltexttype;
    $class->bind_int32type;
    $class->bind_int64type;
    $class->bind_schema;
    $class->bind_stringtype;
}

sub bind_architecture {
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
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
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_blobtype {
    my $synopsis = <<'END_SYNOPSIS';
    my $string_type = Lucy::Plan::StringType->new;
    my $blob_type   = Lucy::Plan::BlobType->new( stored => 1 );
    my $schema      = Lucy::Plan::Schema->new;
    $schema->spec_field( name => 'id',   type => $string_type );
    $schema->spec_field( name => 'jpeg', type => $blob_type );
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $blob_type = Lucy::Plan::BlobType->new(
        stored => 1,  # default: false
    );
END_CONSTRUCTOR

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Plan::BlobType",
        bind_constructors => ["new"],
        make_pod          => {
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
        },
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_fieldtype {
    my $synopis = <<'END_SYNOPSIS';

    my @sortable;
    for my $field ( @{ $schema->all_fields } ) {
        my $type = $schema->fetch_type($field);
        next unless $type->sortable;
        push @sortable, $field;
    }

END_SYNOPSIS

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Plan::FieldType",
        bind_methods => [
            qw(
                Get_Boost
                Indexed
                Stored
                Sortable
                Binary
                Compare_Values
                )
        ],
        bind_constructors => ["new|init2"],
        make_pod          => {
            synopsis => $synopis,
            methods  => [
                qw(
                    get_boost
                    indexed
                    stored
                    sortable
                    binary
                    )
            ],
        }
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_float32type {
    my $synopsis = <<'END_SYNOPSIS';
    my $schema       = Lucy::Plan::Schema->new;
    my $float32_type = Lucy::Plan::FloatType->new;
    $schema->spec_field( name => 'intensity', type => $float32_type );
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $float32_type = Lucy::Plan::Float32Type->new(
        indexed  => 0,    # default true
        stored   => 0,    # default true
        sortable => 1,    # default false
    );
END_CONSTRUCTOR

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Plan::Float32Type",
        bind_constructors => ["new|init2"],
        #make_pod          => {
        #    synopsis    => $synopsis,
        #    constructor => { sample => $constructor },
        #},
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_float64type {
    my $synopsis = <<'END_SYNOPSIS';
    my $schema       = Lucy::Plan::Schema->new;
    my $float64_type = Lucy::Plan::FloatType->new;
    $schema->spec_field( name => 'intensity', type => $float64_type );
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $float64_type = Lucy::Plan::Float64Type->new(
        indexed  => 0     # default true
        stored   => 0,    # default true
        sortable => 1,    # default false
    );
END_CONSTRUCTOR

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Plan::Float64Type",
        bind_constructors => ["new|init2"],
        #make_pod          => {
        #    synopsis    => $synopsis,
        #    constructor => { sample => $constructor },
        #},
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_fulltexttype {
    my $synopsis = <<'END_SYNOPSIS';
    my $polyanalyzer = Lucy::Analysis::PolyAnalyzer->new(
        language => 'en',
    );
    my $type = Lucy::Plan::FullTextType->new(
        analyzer => $polyanalyzer,
    );
    my $schema = Lucy::Plan::Schema->new;
    $schema->spec_field( name => 'title',   type => $type );
    $schema->spec_field( name => 'content', type => $type );
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR';
    my $type = Lucy::Plan::FullTextType->new(
        analyzer      => $analyzer,    # required
        boost         => 2.0,          # default: 1.0
        indexed       => 1,            # default: true
        stored        => 1,            # default: true
        sortable      => 1,            # default: false
        highlightable => 1,            # default: false
    );
END_CONSTRUCTOR

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Plan::FullTextType",
        bind_constructors => ["new|init2"],
        bind_methods      => [
            qw(
                Set_Highlightable
                Highlightable
                )
        ],
        make_pod => {
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
            methods     => [
                qw(
                    set_highlightable
                    highlightable
                    )
            ],
        },
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_int32type {
    my $synopsis = <<'END_SYNOPSIS';
    my $schema     = Lucy::Plan::Schema->new;
    my $int32_type = Lucy::Plan::Int32Type->new;
    $schema->spec_field( name => 'count', type => $int32_type );
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $int32_type = Lucy::Plan::Int32Type->new(
        indexed  => 0,    # default true
        stored   => 0,    # default true
        sortable => 1,    # default false
    );
END_CONSTRUCTOR

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Plan::Int32Type",
        bind_constructors => ["new|init2"],
        #make_pod          => {
        #    synopsis    => $synopsis,
        #    constructor => { sample => $constructor },
        #},
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_int64type {
    my $synopsis = <<'END_SYNOPSIS';
    my $schema     = Lucy::Plan::Schema->new;
    my $int64_type = Lucy::Plan::Int64Type->new;
    $schema->spec_field( name => 'count', type => $int64_type );
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $int64_type = Lucy::Plan::Int64Type->new(
        indexed  => 0,    # default true
        stored   => 0,    # default true
        sortable => 1,    # default false
    );
END_CONSTRUCTOR

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Plan::Int64Type",
        bind_constructors => ["new|init2"],
        #make_pod          => {
        #    synopsis    => $synopsis,
        #    constructor => { sample => $constructor },
        #},
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_schema {
    my $synopsis = <<'END_SYNOPSIS';
    use Lucy::Plan::Schema;
    use Lucy::Plan::FullTextType;
    use Lucy::Analysis::PolyAnalyzer;
    
    my $schema = Lucy::Plan::Schema->new;
    my $polyanalyzer = Lucy::Analysis::PolyAnalyzer->new( 
        language => 'en',
    );
    my $type = Lucy::Plan::FullTextType->new(
        analyzer => $polyanalyzer,
    );
    $schema->spec_field( name => 'title',   type => $type );
    $schema->spec_field( name => 'content', type => $type );
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR';
    my $schema = Lucy::Plan::Schema->new;
END_CONSTRUCTOR

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Plan::Schema",
        bind_methods => [
            qw(
                Architecture
                Get_Architecture
                Get_Similarity
                Fetch_Type
                Fetch_Analyzer
                Fetch_Sim
                Num_Fields
                All_Fields
                Spec_Field
                Write
                Eat
                )
        ],
        bind_constructors => [qw( new )],
        make_pod          => {
            methods => [
                qw(
                    spec_field
                    num_fields
                    all_fields
                    fetch_type
                    fetch_sim
                    architecture
                    get_architecture
                    get_similarity
                    )
            ],
            synopsis     => $synopsis,
            constructors => [ { sample => $constructor } ],
        },
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_stringtype {
    my $synopsis = <<'END_SYNOPSIS';
    my $type   = Lucy::Plan::StringType->new;
    my $schema = Lucy::Plan::Schema->new;
    $schema->spec_field( name => 'category', type => $type );
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR';
    my $type = Lucy::Plan::StringType->new(
        boost    => 0.1,    # default: 1.0
        indexed  => 1,      # default: true
        stored   => 1,      # default: true
        sortable => 1,      # default: false
    );
END_CONSTRUCTOR

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Plan::StringType",
        bind_constructors => ["new|init2"],
        make_pod          => {
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
        },
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

1;
