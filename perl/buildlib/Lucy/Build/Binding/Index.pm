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
package Lucy::Build::Binding::Index;
use strict;
use warnings;

our $VERSION = '0.004001';
$VERSION = eval $VERSION;

sub bind_all {
    my $class = shift;
    $class->bind_backgroundmerger;
    $class->bind_datareader;
    $class->bind_datawriter;
    $class->bind_deletionswriter;
    $class->bind_docreader;
    $class->bind_indexmanager;
    $class->bind_indexreader;
    $class->bind_indexer;
    $class->bind_lexicon;
    $class->bind_lexiconreader;
    $class->bind_polyreader;
    $class->bind_scoreposting;
    $class->bind_postinglist;
    $class->bind_postinglistreader;
    $class->bind_postinglistwriter;
    $class->bind_segreader;
    $class->bind_segwriter;
    $class->bind_segment;
    $class->bind_similarity;
    $class->bind_snapshot;
    $class->bind_sortcache;
    $class->bind_sortwriter;
}

sub bind_backgroundmerger {
    my @exposed = qw( Commit Prepare_Commit Optimize );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $bg_merger = Lucy::Index::BackgroundMerger->new(
        index  => '/path/to/index',
    );
    $bg_merger->commit;
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $bg_merger = Lucy::Index::BackgroundMerger->new(
        index   => '/path/to/index',    # required
        manager => $manager             # default: created internally
    );
END_CONSTRUCTOR
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::BackgroundMerger",
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_datareader {
    my @exposed = qw(
        Get_Schema
        Get_Folder
        Get_Snapshot
        Get_Segments
        Get_Segment
        Get_Seg_Tick
        Aggregator
    );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::DataReader",
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_datawriter {
    my @exposed = qw(
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
    );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::DataWriter",
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_deletionswriter {
    my @exposed = qw(
        Delete_By_Term
        Delete_By_Query
        Updated
        Seg_Del_Count
    );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $polyreader  = $del_writer->get_polyreader;
    my $seg_readers = $polyreader->seg_readers;
    for my $seg_reader (@$seg_readers) {
        my $count = $del_writer->seg_del_count( $seg_reader->get_seg_name );
        ...
    }
END_SYNOPSIS
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::DeletionsWriter",
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_docreader {
    my @exposed = qw( Fetch_Doc Aggregator );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $doc_reader = $seg_reader->obtain("Lucy::Index::DocReader");
    my $doc        = $doc_reader->fetch_doc($doc_id);
END_SYNOPSIS
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::DocReader",
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_indexmanager {
    my @exposed = qw(
        Make_Write_Lock
        Recycle
        Set_Folder
        Get_Folder
        Get_Host
        Set_Write_Lock_Timeout
        Get_Write_Lock_Timeout
        Set_Write_Lock_Interval
        Get_Write_Lock_Interval
    );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::IndexManager",
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_indexreader {
    my @exposed = qw(
        Doc_Max
        Doc_Count
        Del_Count
        Seg_Readers
        Offsets
        Fetch
        Obtain
    );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $reader = Lucy::Index::IndexReader->open(
        index => '/path/to/index',
    );
    my $seg_readers = $reader->seg_readers;
    for my $seg_reader (@$seg_readers) {
        my $seg_name = $seg_reader->get_segment->get_name;
        my $num_docs = $seg_reader->doc_max;
        print "Segment $seg_name ($num_docs documents):\n";
        my $doc_reader = $seg_reader->obtain("Lucy::Index::DocReader");
        for my $doc_id ( 1 .. $num_docs ) {
            my $doc = $doc_reader->fetch_doc($doc_id);
            print "  $doc_id: $doc->{title}\n";
        }
    }
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $reader = Lucy::Index::IndexReader->open(
        index    => '/path/to/index', # required
        snapshot => $snapshot,
        manager  => $index_manager,
    );
END_CONSTRUCTOR
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor(
        alias       => 'open',
        initializer => 'do_open',
        sample      => $constructor,
    );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy    PACKAGE = Lucy::Index::IndexReader

void
set_race_condition_debug1(val_sv)
    SV *val_sv;
PPCODE:
    CFISH_DECREF(lucy_PolyReader_race_condition_debug1);
    lucy_PolyReader_race_condition_debug1 = (cfish_String*)
        XSBind_maybe_sv_to_cfish_obj(val_sv, CFISH_STRING, NULL);
    if (lucy_PolyReader_race_condition_debug1) {
        (void)CFISH_INCREF(lucy_PolyReader_race_condition_debug1);
    }

int32_t
debug1_num_passes()
CODE:
    RETVAL = lucy_PolyReader_debug1_num_passes;
OUTPUT: RETVAL
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::IndexReader",
    );
    $binding->bind_constructor(
        alias       => 'open',
        initializer => 'do_open',
    );
    $binding->exclude_constructor;
    $binding->bind_method( alias => '_offsets', method => 'Offsets' );
    $binding->append_xs($xs_code);
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_indexer {
    my @exposed = qw(
        Add_Index
        Optimize
        Commit
        Prepare_Commit
        Delete_By_Term
        Delete_By_Query
        Delete_By_Doc_ID
        Get_Schema
    );
    my @hand_rolled = qw( Add_Doc );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $indexer = Lucy::Index::Indexer->new(
        schema => $schema,
        index  => '/path/to/index',
        create => 1,
    );
    while ( my ( $title, $content ) = each %source_docs ) {
        $indexer->add_doc({
            title   => $title,
            content => $content,
        });
    }
    $indexer->commit;
END_SYNOPSIS
    my $constructor = <<'END_NEW';
=head2 new( I<[labeled params]> )

    my $indexer = Lucy::Index::Indexer->new(
        schema   => $schema,             # required at index creation
        index    => '/path/to/index',    # required
        create   => 1,                   # default: 0
        truncate => 1,                   # default: 0
        manager  => $manager             # default: created internally
    );

=over

=item *

B<schema> - A Schema.  Required when index is being created; if not supplied,
will be extracted from the index folder.

=item *

B<index> - Either a filepath to an index or a Folder.

=item *

B<create> - If true and the index directory does not exist, attempt to create
it.

=item *

B<truncate> - If true, proceed with the intention of discarding all previous
indexing data.  The old data will remain intact and visible until commit()
succeeds.

=item *

B<manager> - An IndexManager.

=back
END_NEW
    my $add_doc_pod = <<'END_ADD_DOC_POD';
=head2 add_doc(...)

    $indexer->add_doc($doc);
    $indexer->add_doc( { field_name => $field_value } );
    $indexer->add_doc(
        doc   => { field_name => $field_value },
        boost => 2.5,         # default: 1.0
    );

Add a document to the index.  Accepts either a single argument or labeled
params.

=over

=item *

B<doc> - Either a Lucy::Document::Doc object, or a hashref (which will
be attached to a Lucy::Document::Doc object internally).

=item *

B<boost> - A floating point weight which affects how this document scores.

=back

END_ADD_DOC_POD
    $pod_spec->set_synopsis($synopsis);

    # Override necessary because of different handling for flags.
    $pod_spec->add_constructor( alias => 'new', pod => $constructor );

    # Override is necessary because there's no standard way to explain
    # hash/hashref across multiple host languages.
    $pod_spec->add_method(
        method => 'Add_Doc',
        alias  => 'add_doc',
        pod    => $add_doc_pod,
    );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy  PACKAGE = Lucy::Index::Indexer

int32_t
CREATE(...)
CODE:
    CFISH_UNUSED_VAR(items);
    RETVAL = lucy_Indexer_CREATE;
OUTPUT: RETVAL

int32_t
TRUNCATE(...)
CODE:
    CFISH_UNUSED_VAR(items);
    RETVAL = lucy_Indexer_TRUNCATE;
OUTPUT: RETVAL

void
add_doc(self, ...)
    lucy_Indexer *self;
PPCODE:
{
    lucy_Doc *doc = NULL;
    SV *doc_sv = NULL;
    float boost = 1.0;

    if (items == 2) {
        doc_sv = ST(1);
    }
    else if (items > 2) {
        bool args_ok
            = XSBind_allot_params(&(ST(0)), 1, items,
                                  ALLOT_SV(&doc_sv, "doc", 3, true),
                                  ALLOT_F32(&boost, "boost", 5, false),
                                  NULL);
        if (!args_ok) {
            CFISH_RETHROW(CFISH_INCREF(cfish_Err_get_error()));
        }
    }
    else if (items == 1) {
        CFISH_THROW(CFISH_ERR, "Missing required argument 'doc'");
    }

    // Either get a Doc or use the stock doc.
    if (sv_isobject(doc_sv)
        && sv_derived_from(doc_sv, "Lucy::Document::Doc")
       ) {
        IV tmp = SvIV(SvRV(doc_sv));
        doc = INT2PTR(lucy_Doc*, tmp);
    }
    else if (XSBind_sv_defined(doc_sv) && SvROK(doc_sv)) {
        HV *maybe_fields = (HV*)SvRV(doc_sv);
        if (SvTYPE((SV*)maybe_fields) == SVt_PVHV) {
            doc = LUCY_Indexer_Get_Stock_Doc(self);
            LUCY_Doc_Set_Fields(doc, maybe_fields);
        }
    }
    if (!doc) {
        THROW(CFISH_ERR, "Need either a hashref or a %o",
              CFISH_Class_Get_Name(LUCY_DOC));
    }

    LUCY_Indexer_Add_Doc(self, doc, boost);
}
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::Indexer",
    );
    $binding->bind_constructor( alias => '_new' );
    $binding->exclude_method($_) for @hand_rolled;
    $binding->append_xs($xs_code);
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_lexicon {
    my @exposed = qw(
        Seek
        Next
        Get_Term
        Reset
    );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $lex_reader = $seg_reader->obtain('Lucy::Index::LexiconReader');
    my $lexicon = $lex_reader->lexicon( field => 'content' );
    while ( $lexicon->next ) {
       print $lexicon->get_term . "\n";
    }
END_SYNOPSIS
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::Lexicon",
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_lexiconreader {
    my @exposed = qw( Lexicon Doc_Freq );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $lex_reader = $seg_reader->obtain("Lucy::Index::LexiconReader");
    my $lexicon    = $lex_reader->lexicon( field => 'title' );
END_SYNOPSIS
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::LexiconReader",
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_polyreader {
    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);

    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Index::PolyReader

uint32_t
sub_tick(offsets, doc_id)
    lucy_I32Array *offsets;
    int32_t doc_id;
CODE:
    RETVAL = lucy_PolyReader_sub_tick(offsets, doc_id);
OUTPUT: RETVAL

END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::PolyReader",
    );
    $binding->bind_constructor( alias => 'open', initializer => 'do_open' );
    $binding->append_xs($xs_code);
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_scoreposting {
    my @hand_rolled = qw( Get_Prox );

    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Index::Posting::ScorePosting

SV*
get_prox(self)
    lucy_ScorePosting *self;
CODE:
{
    AV *out_av            = newAV();
    uint32_t *positions  = LUCY_ScorePost_Get_Prox(self);
    uint32_t i, max;

    for (i = 0, max = LUCY_ScorePost_Get_Freq(self); i < max; i++) {
        SV *pos_sv = newSVuv(positions[i]);
        av_push(out_av, pos_sv);
    }

    RETVAL = newRV_noinc((SV*)out_av);
}
OUTPUT: RETVAL
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::Posting::ScorePosting",
    );
    $binding->append_xs($xs_code);
    $binding->exclude_method($_) for @hand_rolled;

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_postinglist {
    my @exposed = qw(
        Next
        Advance
        Get_Doc_ID
        Get_Doc_Freq
        Seek
    );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::PostingList",
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_postinglistreader {
    my @exposed = qw( Posting_List );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $posting_list_reader 
        = $seg_reader->obtain("Lucy::Index::PostingListReader");
    my $posting_list = $posting_list_reader->posting_list(
        field => 'title', 
        term  => 'foo',
    );
END_SYNOPSIS
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::PostingListReader",
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_postinglistwriter {
    my $xs_code = <<'END_XS';
MODULE = Lucy    PACKAGE = Lucy::Index::PostingListWriter

void
set_default_mem_thresh(mem_thresh)
    size_t mem_thresh;
PPCODE:
    lucy_PListWriter_set_default_mem_thresh(mem_thresh);
END_XS

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::PostingListWriter",
    );
    $binding->append_xs($xs_code);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_segreader {
    my @exposed = qw(
        Get_Seg_Name
        Get_Seg_Num
    );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::SegReader",
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_segwriter {
    my @exposed = qw(
        Add_Doc
        Add_Writer
        Register
        Fetch
    );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::SegWriter",
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_segment {
    my @exposed = qw(
        Add_Field
        Store_Metadata
        Fetch_Metadata
        Field_Num
        Field_Name
        Get_Name
        Get_Number
        Set_Count
        Get_Count
    );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    # Index-time.
    package MyDataWriter;
    use base qw( Lucy::Index::DataWriter );

    sub finish {
        my $self     = shift;
        my $segment  = $self->get_segment;
        my $metadata = $self->SUPER::metadata();
        $metadata->{foo} = $self->get_foo;
        $segment->store_metadata(
            key       => 'my_component',
            metadata  => $metadata
        );
    }

    # Search-time.
    package MyDataReader;
    use base qw( Lucy::Index::DataReader );

    sub new {
        my $self     = shift->SUPER::new(@_);
        my $segment  = $self->get_segment;
        my $metadata = $segment->fetch_metadata('my_component');
        if ($metadata) {
            $self->set_foo( $metadata->{foo} );
            ...
        }
        return $self;
    }
END_SYNOPSIS
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::Segment",
    );
    $binding->bind_method(
        alias  => '_store_metadata',
        method => 'Store_Metadata',
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_similarity {
    my @exposed = qw(
        Length_Norm
    );
    my @hand_rolled = qw( Get_Norm_Decoder );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    package MySimilarity;

    sub length_norm { return 1.0 }    # disable length normalization

    package MyFullTextType;
    use base qw( Lucy::Plan::FullTextType );

    sub make_similarity { MySimilarity->new }
END_SYNOPSIS
    my $constructor = qq|    my \$sim = Lucy::Index::Similarity->new;\n|;
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy    PACKAGE = Lucy::Index::Similarity

SV*
get_norm_decoder(self)
    lucy_Similarity *self;
CODE:
    RETVAL = newSVpvn((char*)LUCY_Sim_Get_Norm_Decoder(self),
                      (256 * sizeof(float)));
OUTPUT: RETVAL
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::Similarity",
    );
    $binding->bind_method(
        alias  => '_load',
        method => 'Load',
    );
    $binding->exclude_method($_) for @hand_rolled;
    $binding->append_xs($xs_code);
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_snapshot {
    my @exposed = qw(
        List
        Num_Entries
        Add_Entry
        Delete_Entry
        Read_File
        Write_File
        Set_Path
        Get_Path
    );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $snapshot = Lucy::Index::Snapshot->new;
    $snapshot->read_file( folder => $folder );    # load most recent snapshot
    my $files = $snapshot->list;
    print "$_\n" for @$files;
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $snapshot = Lucy::Index::Snapshot->new;
END_CONSTRUCTOR
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::Snapshot",
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_sortcache {
    my @hand_rolled = qw( Value );

    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Index::SortCache

SV*
value(self, ...)
    lucy_SortCache *self;
CODE:
{
    int32_t ord = 0;
    bool args_ok
        = XSBind_allot_params(&(ST(0)), 1, items,
                              ALLOT_I32(&ord, "ord", 3, false),
                              NULL);
    if (!args_ok) {
        CFISH_RETHROW(CFISH_INCREF(cfish_Err_get_error()));
    }
    {
        cfish_Obj *value = LUCY_SortCache_Value(self, ord);
        RETVAL = XSBind_cfish_to_perl(value);
        CFISH_DECREF(value);
    }
}
OUTPUT: RETVAL
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::SortCache",
    );
    $binding->exclude_method($_) for @hand_rolled;
    $binding->append_xs($xs_code);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_sortwriter {
    my $xs_code = <<'END_XS';
MODULE = Lucy    PACKAGE = Lucy::Index::SortWriter

void
set_default_mem_thresh(mem_thresh)
    size_t mem_thresh;
PPCODE:
    lucy_SortWriter_set_default_mem_thresh(mem_thresh);
END_XS

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Index::SortWriter",
    );
    $binding->append_xs($xs_code);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

1;
