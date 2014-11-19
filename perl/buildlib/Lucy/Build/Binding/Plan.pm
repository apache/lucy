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
use strict;
use warnings;

our $VERSION = '0.004002';
$VERSION = eval $VERSION;

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
    my @exposed = qw( Register_Doc_Writer Register_Doc_Reader );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Plan::Architecture",
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_blobtype {
    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Plan::BlobType",
    );
    $binding->set_pod_spec($pod_spec);
    $binding->add_class_alias('KinoSearch::Plan::BlobType'); 
    $binding->add_class_alias('KinoSearch::FieldType::BlobType');

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_fieldtype {
    my @exposed = qw(
        Get_Boost
        Indexed
        Stored
        Sortable
        Binary
    );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my @sortable;
    for my $field ( @{ $schema->all_fields } ) {
        my $type = $schema->fetch_type($field);
        next unless $type->sortable;
        push @sortable, $field;
    }
END_SYNOPSIS
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Plan::FieldType",
    );
    $binding->bind_constructor( alias => 'new', initializer => 'init2' );
    $binding->bind_method(
        alias  => '_load',
        method => 'Load',
    );
    $binding->set_pod_spec($pod_spec);
    $binding->add_class_alias("KinoSearch::Plan::FieldType");
    $binding->add_class_alias("KinoSearch::FieldType");

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_float32type {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Plan::Float32Type",
    );
    $binding->bind_constructor( alias => 'new', initializer => 'init2' );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_float64type {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Plan::Float64Type",
    );
    $binding->bind_constructor( alias => 'new', initializer => 'init2' );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_fulltexttype {
    my @exposed = qw(
        Set_Highlightable
        Highlightable
    );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $easyanalyzer = Lucy::Analysis::EasyAnalyzer->new(
        language => 'en',
    );
    my $type = Lucy::Plan::FullTextType->new(
        analyzer => $easyanalyzer,
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Plan::FullTextType",
    );
    $binding->bind_constructor( alias => 'new', initializer => 'init2' );
    $binding->set_pod_spec($pod_spec);
    $binding->add_class_alias('KinoSearch::Plan::FullTextType');
    $binding->add_class_alias('KinoSearch::FieldType::FullTextType');

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_int32type {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Plan::Int32Type",
    );
    $binding->bind_constructor( alias => 'new', initializer => 'init2' );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_int64type {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Plan::Int64Type",
    );
    $binding->bind_constructor( alias => 'new', initializer => 'init2' );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_schema {
    my @exposed = qw(
        Spec_Field
        Num_Fields
        All_Fields
        Fetch_Type
        Fetch_Sim
        Architecture
        Get_Architecture
        Get_Similarity
    );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    use Lucy::Plan::Schema;
    use Lucy::Plan::FullTextType;
    use Lucy::Analysis::EasyAnalyzer;
    
    my $schema = Lucy::Plan::Schema->new;
    my $easyanalyzer = Lucy::Analysis::EasyAnalyzer->new(
        language => 'en',
    );
    my $type = Lucy::Plan::FullTextType->new(
        analyzer => $easyanalyzer,
    );
    $schema->spec_field( name => 'title',   type => $type );
    $schema->spec_field( name => 'content', type => $type );
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $schema = Lucy::Plan::Schema->new;
END_CONSTRUCTOR
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Plan::Schema",
    );
    $binding->bind_method(
        alias  => '_load',
        method => 'Load',
    );
    $binding->set_pod_spec($pod_spec);
    $binding->add_class_alias("KinoSearch::Plan::Schema");
    $binding->add_class_alias("KinoSearch::Schema");

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_stringtype {
    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Plan::StringType",
    );
    $binding->bind_constructor( alias => 'new', initializer => 'init2' );
    $binding->set_pod_spec($pod_spec);
    $binding->add_class_alias('KinoSearch::Plan::StringType');
    $binding->add_class_alias('KinoSearch::FieldType::StringType');

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

1;
