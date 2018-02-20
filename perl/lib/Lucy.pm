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

use strict;
use warnings;

package Lucy;

use 5.008003;

our $VERSION = '0.006002';
$VERSION = eval $VERSION;
our $MAJOR_VERSION = 0.006000;

use Clownfish 0.006000;
BEGIN {
    die <<"EOF" if $Clownfish::MAJOR_VERSION > 0.006000;
This version of Lucy doesn't support Clownfish $Clownfish::MAJOR_VERSION or
higher. You should downgrade Clownfish or, if possible, upgrade Lucy.
EOF
}

use Exporter 'import';
BEGIN {
    our @EXPORT_OK = qw(
        STORABLE_freeze
        STORABLE_thaw
        );
}

# On most UNIX variants, this flag makes DynaLoader pass RTLD_GLOBAL to
# dl_open, so extensions can resolve the needed symbols without explicitly
# linking against the DSO.
sub dl_load_flags { 1 }

BEGIN {
    require DynaLoader;
    our @ISA = qw( DynaLoader );
    # This loads a large number of disparate subs.
    bootstrap Lucy '0.6.2';
}

{
    package Lucy::Util::IndexFileNames;
    our $VERSION = '0.006002';
    $VERSION = eval $VERSION;
    BEGIN {
        push our @ISA, 'Exporter';
        our @EXPORT_OK = qw(
            extract_gen
            latest_snapshot
        );
    }
}

{
    # Temporary back compat.
    package Lucy::Object::Obj;
    BEGIN { our @ISA = qw( Clownfish::Obj ) }
}

{
    package Lucy::Analysis::RegexTokenizer;
    our $VERSION = '0.006002';
    $VERSION = eval $VERSION;

    sub _compile_token_re {qr/$_[0]/}

    sub new {
        my ( $either, %args ) = @_;
        my $token_re = delete $args{token_re};
        $args{pattern} = "$token_re" if $token_re;
        return $either->_new(%args);
    }
}

{
    package Lucy::Document::Doc;
    our $VERSION = '0.006002';
    $VERSION = eval $VERSION;
    use Storable ();  # Needed by serialize/deserialize.
    use Lucy qw( STORABLE_freeze STORABLE_thaw );

    use overload
        fallback => 1,
        '%{}'    => \&get_fields;
}

{
    package Lucy::Index::DocVector;
    our $VERSION = '0.006002';
    $VERSION = eval $VERSION;
    use Lucy qw( STORABLE_freeze STORABLE_thaw );
}

{
    package Lucy::Index::Indexer;
    our $VERSION = '0.006002';
    $VERSION = eval $VERSION;

    sub new {
        my ( $either, %args ) = @_;
        my $flags = 0;
        $flags |= CREATE   if delete $args{'create'};
        $flags |= TRUNCATE if delete $args{'truncate'};
        return $either->_new( %args, flags => $flags );
    }
}

{
    package Lucy::Index::IndexReader;
    our $VERSION = '0.006002';
    $VERSION = eval $VERSION;
    use Carp;

    sub new {
        confess(
            "IndexReader is an abstract class; use open() instead of new()");
    }
    sub lexicon {
        my $self       = shift;
        my $lex_reader = $self->fetch("Lucy::Index::LexiconReader");
        return $lex_reader->lexicon(@_) if $lex_reader;
        return;
    }
    sub posting_list {
        my $self         = shift;
        my $plist_reader = $self->fetch("Lucy::Index::PostingListReader");
        return $plist_reader->posting_list(@_) if $plist_reader;
        return;
    }
    sub offsets { shift->_offsets->to_arrayref }
}

{
    package Lucy::Index::Similarity;
    our $VERSION = '0.006002';
    $VERSION = eval $VERSION;
    use Lucy qw( STORABLE_freeze STORABLE_thaw );
}

{
    package Lucy::Index::TermVector;
    our $VERSION = '0.006002';
    $VERSION = eval $VERSION;
    use Lucy qw( STORABLE_freeze STORABLE_thaw );
}

{
    package Lucy::Search::Compiler;
    our $VERSION = '0.006002';
    $VERSION = eval $VERSION;
    use Carp;
    use Scalar::Util qw( blessed );

    sub new {
        my ( $either, %args ) = @_;
        if ( !defined $args{boost} ) {
            confess("'parent' is not a Query")
                unless ( blessed( $args{parent} )
                and $args{parent}->isa("Lucy::Search::Query") );
            $args{boost} = $args{parent}->get_boost;
        }
        return $either->do_new(%args);
    }
}

{
    package Lucy::Search::MatchDoc;
    our $VERSION = '0.006002';
    $VERSION = eval $VERSION;
    use Lucy qw( STORABLE_freeze STORABLE_thaw );
}

{
    package Lucy::Search::Query;
    our $VERSION = '0.006002';
    $VERSION = eval $VERSION;
    use Lucy qw( STORABLE_freeze STORABLE_thaw );
}

{
    package Lucy::Search::SortRule;
    our $VERSION = '0.006002';
    $VERSION = eval $VERSION;
    use Carp;
    use Lucy qw( STORABLE_freeze STORABLE_thaw );

    my %types = (
        field  => FIELD(),
        score  => SCORE(),
        doc_id => DOC_ID(),
    );

    sub new {
        my ( $either, %args ) = @_;
        my $type = delete $args{type} || 'field';
        confess("Invalid type: '$type'") unless defined $types{$type};
        return $either->_new( %args, type => $types{$type} );
    }
}

{
    package Lucy::Search::SortSpec;
    our $VERSION = '0.006002';
    $VERSION = eval $VERSION;
    use Lucy qw( STORABLE_freeze STORABLE_thaw );
}

{
    package Lucy::Search::TopDocs;
    our $VERSION = '0.006002';
    $VERSION = eval $VERSION;
    use Lucy qw( STORABLE_freeze STORABLE_thaw );
}

{
    package Lucy::Object::BitVector;
    our $VERSION = '0.006002';
    $VERSION = eval $VERSION;
    sub to_arrayref { shift->to_array->to_arrayref }
}

{
    package Lucy::Store::FileHandle;
    our $VERSION = '0.006002';
    $VERSION = eval $VERSION;
    BEGIN {
        push our @ISA, 'Exporter';
        our @EXPORT_OK = qw( build_fh_flags );
    }

    sub build_fh_flags {
        my $args  = shift;
        my $flags = 0;
        $flags |= FH_CREATE     if delete $args->{create};
        $flags |= FH_READ_ONLY  if delete $args->{read_only};
        $flags |= FH_WRITE_ONLY if delete $args->{write_only};
        $flags |= FH_EXCLUSIVE  if delete $args->{exclusive};
        return $flags;
    }

    sub open {
        my ( $either, %args ) = @_;
        $args{flags} ||= 0;
        $args{flags} |= build_fh_flags( \%args );
        return $either->_open(%args);
    }
}

{
    package Lucy::Store::FSFileHandle;
    our $VERSION = '0.006002';
    $VERSION = eval $VERSION;

    sub open {
        my ( $either, %args ) = @_;
        $args{flags} ||= 0;
        $args{flags} |= Lucy::Store::FileHandle::build_fh_flags( \%args );
        return $either->_open(%args);
    }
}

{
    package Lucy::Store::RAMFileHandle;
    our $VERSION = '0.006002';
    $VERSION = eval $VERSION;

    sub open {
        my ( $either, %args ) = @_;
        $args{flags} ||= 0;
        $args{flags} |= Lucy::Store::FileHandle::build_fh_flags( \%args );
        return $either->_open(%args);
    }
}

{
    package Lucy::Util::Debug;
    our $VERSION = '0.006002';
    $VERSION = eval $VERSION;
    BEGIN {
        push our @ISA, 'Exporter';
        our @EXPORT_OK = qw(
            DEBUG
            DEBUG_PRINT
            DEBUG_ENABLED
            ASSERT
            set_env_cache
            num_allocated
            num_freed
            num_globals
        );
    }
}

{
    package Lucy::Util::StringHelper;
    our $VERSION = '0.006002';
    $VERSION = eval $VERSION;
    BEGIN {
        push our @ISA, 'Exporter';
        our @EXPORT_OK = qw(
            utf8_flag_on
            utf8_flag_off
            to_base36
            utf8ify
            utf8_valid
            cat_bytes
        );
    }
}

1;

__END__


