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

sub bind_all {
    my $class = shift;
    $class->bind_backgroundmerger;
    $class->bind_datareader;
    $class->bind_datawriter;
    $class->bind_deletionsreader;
    $class->bind_defaultdeletionsreader;
    $class->bind_deletionswriter;
    $class->bind_defaultdeletionswriter;
    $class->bind_docreader;
    $class->bind_defaultdocreader;
    $class->bind_docvector;
    $class->bind_docwriter;
    $class->bind_filepurger;
    $class->bind_highlightreader;
    $class->bind_defaulthighlightreader;
    $class->bind_highlightwriter;
    $class->bind_indexmanager;
    $class->bind_indexreader;
    $class->bind_indexer;
    $class->bind_inverter;
    $class->bind_lexicon;
    $class->bind_lexiconreader;
    $class->bind_defaultlexiconreader;
    $class->bind_lexiconwriter;
    $class->bind_polylexicon;
    $class->bind_polyreader;
    $class->bind_posting;
    $class->bind_postinglist;
    $class->bind_postinglistreader;
    $class->bind_defaultpostinglistreader;
    $class->bind_postinglistwriter;
    $class->bind_seglexicon;
    $class->bind_segpostinglist;
    $class->bind_segreader;
    $class->bind_segwriter;
    $class->bind_segment;
    $class->bind_similarity;
    $class->bind_snapshot;
    $class->bind_sortcache;
    $class->bind_sortreader;
    $class->bind_defaultsortreader;
    $class->bind_sortwriter;
    $class->bind_terminfo;
    $class->bind_termvector;
}

sub bind_backgroundmerger {
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Index::BackgroundMerger",
        make_pod          => {
            methods => [
                qw(
                    commit
                    prepare_commit
                    optimize
                    )
            ],
            synopsis     => $synopsis,
            constructors => [ { sample => $constructor } ],
        },
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Commit
        Prepare_Commit
        Optimize
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_datareader {
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Index::DataReader",
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
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Get_Schema
        Get_Folder
        Get_Segments
        Get_Snapshot
        Get_Seg_Tick
        Get_Segment
        Aggregator
        Close
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_datawriter {
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Index::DataWriter",
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
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
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
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_deletionsreader {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::DeletionsReader",
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw( Iterator Del_Count );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_defaultdeletionsreader {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::DefaultDeletionsReader",
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw( Read_Deletions );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_deletionswriter {
    my $synopsis = <<'END_SYNOPSIS';
    my $polyreader  = $del_writer->get_polyreader;
    my $seg_readers = $polyreader->seg_readers;
    for my $seg_reader (@$seg_readers) {
        my $count = $del_writer->seg_del_count( $seg_reader->get_seg_name );
        ...
    }
END_SYNOPSIS

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Index::DeletionsWriter",
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
    $binding->bind_method( method => $_ ) for qw(
        Generate_Doc_Map
        Delete_By_Term
        Delete_By_Query
        Delete_By_Doc_ID
        Updated
        Seg_Deletions
        Seg_Del_Count
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_defaultdeletionswriter {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::DefaultDeletionsWriter",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_docreader {
    my $synopsis = <<'END_SYNOPSIS';
    my $doc_reader = $seg_reader->obtain("Lucy::Index::DocReader");
    my $doc        = $doc_reader->fetch_doc($doc_id);
END_SYNOPSIS

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::DocReader",
        make_pod          => {
            synopsis => $synopsis,
            methods  => [qw( fetch_doc aggregator )],
        },
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw( Fetch_Doc );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_defaultdocreader {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::DefaultDocReader",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_docvector {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::DocVector",
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Term_Vector
        Field_Buf
        Add_Field_Buf
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_docwriter {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::DocWriter",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_filepurger {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::FilePurger",
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw( Purge );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_highlightreader {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::HighlightReader",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_defaulthighlightreader {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::DefaultHighlightReader",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_highlightwriter {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::HighlightWriter",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_indexmanager {
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::IndexManager",
        make_pod => {
            methods => [
                qw(
                    make_write_lock
                    recycle
                    set_folder
                    get_folder
                    get_host
                    set_write_lock_timeout
                    get_write_lock_timeout
                    set_write_lock_interval
                    get_write_lock_interval
                    )
            ],
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
        },
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Recycle
        Make_Write_Lock
        Make_Deletion_Lock
        Make_Merge_Lock
        Make_Snapshot_Read_Lock
        Highest_Seg_Num
        Make_Snapshot_Filename
        Set_Folder
        Get_Folder
        Get_Host
        Set_Write_Lock_Timeout
        Get_Write_Lock_Timeout
        Set_Write_Lock_Interval
        Get_Write_Lock_Interval
        Set_Merge_Lock_Timeout
        Get_Merge_Lock_Timeout
        Set_Merge_Lock_Interval
        Get_Merge_Lock_Interval
        Set_Deletion_Lock_Timeout
        Get_Deletion_Lock_Timeout
        Set_Deletion_Lock_Interval
        Get_Deletion_Lock_Interval
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_indexreader {
    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy    PACKAGE = Lucy::Index::IndexReader

void
set_race_condition_debug1(val_sv)
    SV *val_sv;
PPCODE:
    CFISH_DECREF(lucy_PolyReader_race_condition_debug1);
    lucy_PolyReader_race_condition_debug1 = (lucy_CharBuf*)
        XSBind_maybe_sv_to_cfish_obj(val_sv, LUCY_CHARBUF, NULL);
    if (lucy_PolyReader_race_condition_debug1) {
        (void)CFISH_INCREF(lucy_PolyReader_race_condition_debug1);
    }

int32_t
debug1_num_passes()
CODE:
    RETVAL = lucy_PolyReader_debug1_num_passes;
OUTPUT: RETVAL
END_XS_CODE

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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Index::IndexReader",
        xs_code      => $xs_code,
        make_pod          => {
            synopsis    => $synopsis,
            constructor => {
                name   => 'open',
                func   => 'do_open',
                sample => $constructor,
            },
            methods => [
                qw(
                    doc_max
                    doc_count
                    del_count
                    seg_readers
                    offsets
                    fetch
                    obtain
                    )
            ]
        },
    );
    $binding->bind_constructor(
        alias       => 'open',
        initializer => 'do_open',
    );
    $binding->bind_method( method => $_ ) for qw(
        Doc_Max
        Doc_Count
        Del_Count
        Fetch
        Obtain
        Seg_Readers
        Get_Components
    );
    $binding->bind_method( alias => '_offsets', method => 'Offsets' );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_indexer {
    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy  PACKAGE = Lucy::Index::Indexer

int32_t
CREATE(...)
CODE:
    CHY_UNUSED_VAR(items);
    RETVAL = lucy_Indexer_CREATE;
OUTPUT: RETVAL

int32_t
TRUNCATE(...)
CODE:
    CHY_UNUSED_VAR(items);
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
        chy_bool_t args_ok
            = XSBind_allot_params(&(ST(0)), 1, items,
                                  "Lucy::Index::Indexer::add_doc_PARAMS",
                                  ALLOT_SV(&doc_sv, "doc", 3, true),
                                  ALLOT_F32(&boost, "boost", 5, false),
                                  NULL);
        if (!args_ok) {
            CFISH_RETHROW(CFISH_INCREF(cfish_Err_get_error()));
        }
    }
    else if (items == 1) {
        CFISH_THROW(LUCY_ERR, "Missing required argument 'doc'");
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
            doc = Lucy_Indexer_Get_Stock_Doc(self);
            Lucy_Doc_Set_Fields(doc, maybe_fields);
        }
    }
    if (!doc) {
        THROW(LUCY_ERR, "Need either a hashref or a %o",
              Lucy_VTable_Get_Name(LUCY_DOC));
    }

    Lucy_Indexer_Add_Doc(self, doc, boost);
}
END_XS_CODE

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

    # Override is necessary because there's no standard way to explain
    # hash/hashref across multiple host languages.
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Index::Indexer",
        xs_code      => $xs_code,
        make_pod          => {
            methods => [
                { name => 'add_doc', pod => $add_doc_pod },
                qw(
                    add_index
                    optimize
                    commit
                    prepare_commit
                    delete_by_term
                    delete_by_query
                    )
            ],
            synopsis     => $synopsis,
            constructors => [ { pod => $constructor } ],
        },
    );
    $binding->bind_constructor( alias => '_new' );
    $binding->bind_method( method => $_ ) for qw(
        Delete_By_Term
        Delete_By_Query
        Add_Index
        Commit
        Prepare_Commit
        Optimize
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_inverter {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::Inverter",
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Get_Doc
        Iterate
        Next
        Clear
        Get_Field_Name
        Get_Value
        Get_Type
        Get_Analyzer
        Get_Similarity
        Get_Inversion
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_lexicon {
    my $synopsis = <<'END_SYNOPSIS';
    my $lex_reader = $seg_reader->obtain('Lucy::Index::LexiconReader');
    my $lexicon = $lex_reader->lexicon( field => 'content' );
    while ( $lexicon->next ) {
       print $lexicon->get_term . "\n";
    }
END_SYNOPSIS

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::Lexicon",
        make_pod          => {
            synopsis => $synopsis,
            methods  => [qw( seek next get_term reset )],
        },
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Seek
        Next
        Reset
        Get_Term
        Get_Field
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_lexiconreader {
    my $synopsis = <<'END_SYNOPSIS';
    my $lex_reader = $seg_reader->obtain("Lucy::Index::LexiconReader");
    my $lexicon    = $lex_reader->lexicon( field => 'title' );
END_SYNOPSIS

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::LexiconReader",
        make_pod          => {
            synopsis => $synopsis,
            methods  => [qw( lexicon doc_freq )],
        },
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Lexicon
        Doc_Freq
        Fetch_Term_Info
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_defaultlexiconreader {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::DefaultLexiconReader",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_lexiconwriter {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::LexiconWriter",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_polylexicon {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::PolyLexicon",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_polyreader {
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
        parcel            => "Lucy",
        class_name        => "Lucy::Index::PolyReader",
        make_pod          => { synopsis => $synopsis },
        xs_code           => $xs_code,
    );
    $binding->bind_constructor;
    $binding->bind_constructor( alias => 'open', initializer => 'do_open' );
    $binding->bind_method( method => $_ ) for qw( Get_Seg_Readers );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_posting {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Index::Posting",
        #    make_pod => {
        #        synopsis => "    # Abstract base class.\n",
        #    },
    );
    $binding->bind_method( method => $_ ) for qw( Get_Doc_ID );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_postinglist {
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Index::PostingList",
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
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Seek
        Get_Posting
        Get_Doc_Freq
        Make_Matcher
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_postinglistreader {
    my $synopsis = <<'END_SYNOPSIS';
    my $posting_list_reader 
        = $seg_reader->obtain("Lucy::Index::PostingListReader");
    my $posting_list = $posting_list_reader->posting_list(
        field => 'title', 
        term  => 'foo',
    );
END_SYNOPSIS

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::PostingListReader",
        make_pod          => {
            synopsis => $synopsis,
            methods  => [qw( posting_list )],
        },
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Posting_List
        Get_Lex_Reader
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_defaultpostinglistreader {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::DefaultPostingListReader",
    );
    $binding->bind_constructor;
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
        parcel            => "Lucy",
        class_name        => "Lucy::Index::PostingListWriter",
        xs_code           => $xs_code,
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_seglexicon {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::SegLexicon",
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Get_Term_Info
        Get_Field_Num
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_segpostinglist {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::SegPostingList",
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw( Get_Post_Stream Get_Count );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_segreader {
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::SegReader",
        make_pod          => {
            synopsis => $synopsis,
            methods  => [qw( Get_Seg_Name Get_Seg_Num )],
        },
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Get_Seg_Name
        Get_Seg_Num
        Register
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_segwriter {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::SegWriter",
        make_pod => {
            methods => [
                qw(
                    add_doc
                    add_writer
                    register
                    fetch
                    )
            ],
        }
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Add_Writer
        Register
        Fetch
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_segment {
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Index::Segment",
        make_pod          => {
            synopsis => $synopsis,
            methods  => [
                qw(
                    add_field
                    store_metadata
                    fetch_metadata
                    field_num
                    field_name
                    get_name
                    get_number
                    set_count
                    get_count
                    )
            ],
        },
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Add_Field
        Fetch_Metadata
        Field_Num
        Field_Name
        Get_Name
        Get_Number
        Set_Count
        Get_Count
        Write_File
        Read_File
    );
    $binding->bind_method(
        alias  => '_store_metadata',
        method => 'Store_Metadata',
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_similarity {
    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy    PACKAGE = Lucy::Index::Similarity

SV*
get_norm_decoder(self)
    lucy_Similarity *self;
CODE:
    RETVAL = newSVpvn((char*)Lucy_Sim_Get_Norm_Decoder(self),
                      (256 * sizeof(float)));
OUTPUT: RETVAL
END_XS_CODE

    my $synopsis = <<'END_SYNOPSIS';
    package MySimilarity;

    sub length_norm { return 1.0 }    # disable length normalization

    package MyFullTextType;
    use base qw( Lucy::Plan::FullTextType );

    sub make_similarity { MySimilarity->new }
END_SYNOPSIS

    my $constructor = qq|    my \$sim = Lucy::Index::Similarity->new;\n|;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Index::Similarity",
        xs_code      => $xs_code,
        make_pod          => {
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
            methods     => [qw( length_norm )],
        }
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        IDF
        TF
        Encode_Norm
        Decode_Norm
        Query_Norm
        Length_Norm
        Coord
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_snapshot {
    my $synopsis = <<'END_SYNOPSIS';
    my $snapshot = Lucy::Index::Snapshot->new;
    $snapshot->read_file( folder => $folder );    # load most recent snapshot
    my $files = $snapshot->list;
    print "$_\n" for @$files;
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR';
    my $snapshot = Lucy::Index::Snapshot->new;
END_CONSTRUCTOR

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Index::Snapshot",
        make_pod          => {
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
            methods     => [
                qw(
                    list
                    num_entries
                    add_entry
                    delete_entry
                    read_file
                    write_file
                    set_path
                    get_path
                    )
            ],
        },
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        List
        Num_Entries
        Add_Entry
        Delete_Entry
        Read_File
        Write_File
        Set_Path
        Get_Path
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_sortcache {
    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Index::SortCache

SV*
value(self, ...)
    lucy_SortCache *self;
CODE:
{
    int32_t ord = 0;
    chy_bool_t args_ok
        = XSBind_allot_params(&(ST(0)), 1, items,
                              "Lucy::Index::SortCache::value_PARAMS",
                              ALLOT_I32(&ord, "ord", 3, false),
                              NULL);
    if (!args_ok) {
        CFISH_RETHROW(CFISH_INCREF(cfish_Err_get_error()));
    }
    {
        lucy_Obj *blank = Lucy_SortCache_Make_Blank(self);
        lucy_Obj *value = Lucy_SortCache_Value(self, ord, blank);
        RETVAL = XSBind_cfish_to_perl(value);
        CFISH_DECREF(blank);
    }
}
OUTPUT: RETVAL
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Index::SortCache",
        xs_code      => $xs_code,
    );
    $binding->bind_method( method => $_ ) for qw( Ordinal Find );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_sortreader {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::SortReader",
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw( Fetch_Sort_Cache );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_defaultsortreader {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::DefaultSortReader",
    );
    $binding->bind_constructor;
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
        parcel            => "Lucy",
        class_name        => "Lucy::Index::SortWriter",
        xs_code           => $xs_code,
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_terminfo {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Index::TermInfo",
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Get_Doc_Freq
        Get_Lex_FilePos
        Get_Post_FilePos
        Get_Skip_FilePos
        Set_Doc_Freq
        Set_Lex_FilePos
        Set_Post_FilePos
        Set_Skip_FilePos
        Reset
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_termvector {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Index::TermVector",
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Get_Positions
        Get_Start_Offsets
        Get_End_Offsets
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

1;
