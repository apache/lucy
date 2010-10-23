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

package KinoSearch;

use 5.008003;
use Exporter;

our $VERSION = '0.30_12';
$VERSION = eval $VERSION;

use XSLoader;
# This loads a large number of disparate subs.
BEGIN { XSLoader::load( 'KinoSearch', '0.30_12' ) }

BEGIN {
    push our @ISA, 'Exporter';
    our @EXPORT_OK = qw( to_kino to_perl kdump );
}

use KinoSearch::Autobinding;

sub kdump {
    require Data::Dumper;
    my $kdumper = Data::Dumper->new( [@_] );
    $kdumper->Sortkeys( sub { return [ sort keys %{ $_[0] } ] } );
    $kdumper->Indent(1);
    warn $kdumper->Dump;
}

sub error {$KinoSearch::Object::Err::error}

{
    package KinoSearch::Util::IndexFileNames;
    BEGIN {
        push our @ISA, 'Exporter';
        our @EXPORT_OK = qw(
            extract_gen
            latest_snapshot
        );
    }
}

{
    package KinoSearch::Util::StringHelper;
    BEGIN {
        push our @ISA, 'Exporter';
        our @EXPORT_OK = qw(
            utf8_flag_on
            utf8_flag_off
            to_base36
            from_base36
            utf8ify
            utf8_valid
            cat_bytes
        );
    }
}

{
    # Temporary back compat.
    package KinoSearch::Doc;
    BEGIN { our @ISA = qw( KinoSearch::Document::Doc ) }
    package KinoSearch::Architecture;
    BEGIN { our @ISA = qw( KinoSearch::Plan::Architecture ) }
    package KinoSearch::FieldType;
    BEGIN { our @ISA = qw( KinoSearch::Plan::FieldType ) }
    package KinoSearch::FieldType::BlobType;
    BEGIN { our @ISA = qw( KinoSearch::Plan::BlobType ) }
    package KinoSearch::FieldType::Float32Type;
    BEGIN { our @ISA = qw( KinoSearch::Plan::Float32Type ) }
    package KinoSearch::FieldType::Float64Type;
    BEGIN { our @ISA = qw( KinoSearch::Plan::Float64Type ) }
    package KinoSearch::FieldType::Int32Type;
    BEGIN { our @ISA = qw( KinoSearch::Plan::Int32Type ) }
    package KinoSearch::FieldType::Int64Type;
    BEGIN { our @ISA = qw( KinoSearch::Plan::Int64Type ) }
    package KinoSearch::FieldType::FullTextType;
    BEGIN { our @ISA = qw( KinoSearch::Plan::FullTextType ) }
    package KinoSearch::FieldType::StringType;
    BEGIN { our @ISA = qw( KinoSearch::Plan::StringType ) }
    package KinoSearch::Indexer;
    BEGIN { our @ISA = qw( KinoSearch::Index::Indexer ) }
    package KinoSearch::Obj::BitVector;
    BEGIN { our @ISA = qw( KinoSearch::Object::BitVector ) }
    package KinoSearch::QueryParser;
    BEGIN { our @ISA = qw( KinoSearch::Search::QueryParser ) }
    package KinoSearch::Search::HitCollector;
    BEGIN { our @ISA = qw( KinoSearch::Search::Collector ) }
    package KinoSearch::Search::HitCollector::BitCollector;
    BEGIN { our @ISA = qw( KinoSearch::Search::Collector::BitCollector ) }
    package KinoSearch::Search::Searchable;
    BEGIN { our @ISA = qw( KinoSearch::Search::Searcher ) }
    package KinoSearch::Search::Similarity;
    BEGIN { our @ISA = qw( KinoSearch::Index::Similarity ) }
    package KinoSearch::Schema;
    BEGIN { our @ISA = qw( KinoSearch::Plan::Schema ) }
    package KinoSearch::Searcher;
    BEGIN { our @ISA = qw( KinoSearch::Search::IndexSearcher ) }
    package KinoSearch::Util::BitVector;
    BEGIN { our @ISA = qw( KinoSearch::Object::BitVector ) }
}

{
    package KinoSearch::Analysis::Inversion;

    our %new_PARAMS = (
        # params
        text => undef
    );
}

{
    package KinoSearch::Analysis::Stemmer;
    sub lazy_load_snowball {
        require Lingua::Stem::Snowball;
        KinoSearch::Analysis::Stemmer::_copy_snowball_symbols();
    }
}

{
    package KinoSearch::Analysis::Stopalizer;
    use KinoSearch qw( to_kino );

    sub gen_stoplist {
        my ( undef, $language ) = @_;
        require Lingua::StopWords;
        $language = lc($language);
        if ( $language =~ /^(?:da|de|en|es|fi|fr|hu|it|nl|no|pt|ru|sv)$/ ) {
            my $stoplist
                = Lingua::StopWords::getStopWords( $language, 'UTF-8' );
            return to_kino($stoplist);
        }
        return undef;
    }
}

{
    package KinoSearch::Analysis::Token;

    our %new_PARAMS = (
        text         => undef,
        start_offset => undef,
        end_offset   => undef,
        pos_inc      => 1,
        boost        => 1.0,
    );
}

{
    package KinoSearch::Analysis::Tokenizer;

    sub compile_token_re { return qr/$_[1]/ }

    sub new {
        my ( $either, %args ) = @_;
        my $token_re = delete $args{token_re};
        $args{pattern} = "$token_re" if $token_re;
        return $either->_new(%args);
    }
}

{
    package KinoSearch::Document::Doc;
    use Storable qw( nfreeze thaw );
    use bytes;
    no bytes;

    use overload
        fallback => 1,
        '%{}'    => \&get_fields;

    sub serialize_fields {
        my ( $self, $outstream ) = @_;
        my $buf = nfreeze( $self->get_fields );
        $outstream->write_c32( bytes::length($buf) );
        $outstream->print($buf);
    }

    sub deserialize_fields {
        my ( $self, $instream ) = @_;
        my $len = $instream->read_c32;
        my $buf;
        $instream->read( $buf, $len );
        $self->set_fields( thaw($buf) );
    }
}

{
    package KinoSearch::Object::I32Array;
    our %new_PARAMS = ( ints => undef );
}

{
    package KinoSearch::Object::LockFreeRegistry;
    sub DESTROY { }    # leak all
}

{
    package KinoSearch::Object::Obj;
    use KinoSearch qw( to_kino to_perl );
    sub load { return $_[0]->_load( to_kino( $_[1] ) ) }
}

{
    package KinoSearch::Object::VTable;

    sub find_parent_class {
        my ( undef, $package ) = @_;
        no strict 'refs';
        for my $parent ( @{"$package\::ISA"} ) {
            return $parent if $parent->isa('KinoSearch::Object::Obj');
        }
        return;
    }

    sub novel_host_methods {
        my ( undef, $package ) = @_;
        no strict 'refs';
        my $stash   = \%{"$package\::"};
        my $methods = KinoSearch::Object::VArray->new(
            capacity => scalar keys %$stash );
        while ( my ( $symbol, $glob ) = each %$stash ) {
            next if ref $glob;
            next unless *$glob{CODE};
            $methods->push( KinoSearch::Object::CharBuf->new($symbol) );
        }
        return $methods;
    }

    sub _register {
        my ( undef, %args ) = @_;
        my $singleton_class = $args{singleton}->get_name;
        my $parent_class    = $args{parent}->get_name;
        if ( !$singleton_class->isa($parent_class) ) {
            no strict 'refs';
            push @{"$singleton_class\::ISA"}, $parent_class;
        }
    }
}

{
    package KinoSearch::Index::Indexer;

    sub new {
        my ( $either, %args ) = @_;
        my $flags = 0;
        $flags |= CREATE   if delete $args{'create'};
        $flags |= TRUNCATE if delete $args{'truncate'};
        return $either->_new( %args, flags => $flags );
    }

    our %add_doc_PARAMS = ( doc => undef, boost => 1.0 );
}

{
    package KinoSearch::Index::IndexReader;
    use Carp;

    sub new {
        confess(
            "IndexReader is an abstract class; use open() instead of new()");
    }
    sub lexicon {
        my $self       = shift;
        my $lex_reader = $self->fetch("KinoSearch::Index::LexiconReader");
        return $lex_reader->lexicon(@_) if $lex_reader;
        return;
    }
    sub posting_list {
        my $self = shift;
        my $plist_reader
            = $self->fetch("KinoSearch::Index::PostingListReader");
        return $plist_reader->posting_list(@_) if $plist_reader;
        return;
    }
    sub offsets { shift->_offsets->to_arrayref }
}

{
    package KinoSearch::Index::PolyReader;
    use KinoSearch qw( to_kino );

    sub try_read_snapshot {
        my ( undef, %args ) = @_;
        my ( $snapshot, $folder, $path ) = @args{qw( snapshot folder path )};
        eval { $snapshot->read_file( folder => $folder, path => $path ); };
        if   ($@) { return KinoSearch::Object::CharBuf->new($@) }
        else      { return undef }
    }

    sub try_open_segreaders {
        my ( $self, $segments ) = @_;
        my $schema      = $self->get_schema;
        my $folder      = $self->get_folder;
        my $snapshot    = $self->get_snapshot;
        my $seg_readers = KinoSearch::Object::VArray->new(
            capacity => scalar @$segments );
        my $segs = to_kino($segments);    # FIXME: Don't convert twice.
        eval {
            # Create a SegReader for each segment in the index.
            my $num_segs = scalar @$segments;
            for ( my $seg_tick = 0; $seg_tick < $num_segs; $seg_tick++ ) {
                my $seg_reader = KinoSearch::Index::SegReader->new(
                    schema   => $schema,
                    folder   => $folder,
                    segments => $segs,
                    seg_tick => $seg_tick,
                    snapshot => $snapshot,
                );
                $seg_readers->push($seg_reader);
            }
        };
        if ($@) {
            return KinoSearch::Object::CharBuf->new($@);
        }
        return $seg_readers;
    }
}

{
    package KinoSearch::Index::Segment;
    use KinoSearch qw( to_kino );
    sub store_metadata {
        my ( $self, %args ) = @_;
        $self->_store_metadata( %args,
            metadata => to_kino( $args{metadata} ) );
    }
}

{
    package KinoSearch::Index::SegReader;

    sub try_init_components {
        my $self = shift;
        my $arch = $self->get_schema->get_architecture;
        eval { $arch->init_seg_reader($self); };
        if ($@) { return KinoSearch::Object::CharBuf->new($@); }
        return;
    }
}

{
    package KinoSearch::Index::SortCache;
    our %value_PARAMS = ( ord => undef, );
}

{
    package KinoSearch::Search::Compiler;
    use Carp;
    use Scalar::Util qw( blessed );

    sub new {
        my ( $either, %args ) = @_;
        if ( !defined $args{boost} ) {
            confess("'parent' is not a Query")
                unless ( blessed( $args{parent} )
                and $args{parent}->isa("KinoSearch::Search::Query") );
            $args{boost} = $args{parent}->get_boost;
        }
        return $either->do_new(%args);
    }
}

{
    package KinoSearch::Search::Query;

    sub make_compiler {
        my ( $self, %args ) = @_;
        $args{boost} = $self->get_boost unless defined $args{boost};
        return $self->_make_compiler(%args);
    }
}

{
    package KinoSearch::Search::SortRule;

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
    package KinoSearch::Object::BitVector;
    sub to_arrayref { shift->to_array->to_arrayref }
}

{
    package KinoSearch::Object::ByteBuf;
    {
        # Override autogenerated deserialize binding.
        no warnings 'redefine';
        sub deserialize { shift->_deserialize(@_) }
    }
}

{
    package KinoSearch::Object::ViewByteBuf;
    use Carp;
    sub new { confess "ViewByteBuf objects can only be created from C." }
}

{
    package KinoSearch::Object::CharBuf;

    {
        # Defeat obscure bugs in the XS auto-generation by redefining clone()
        # and deserialize().  (Because of how the typemap works for CharBuf*,
        # the auto-generated methods return UTF-8 Perl scalars rather than
        # actual CharBuf objects.)
        no warnings 'redefine';
        sub clone       { shift->_clone(@_) }
        sub deserialize { shift->_deserialize(@_) }
    }
}

{
    package KinoSearch::Object::ViewCharBuf;
    use Carp;
    sub new { confess "ViewCharBuf has no public constructor." }
}

{
    package KinoSearch::Object::ZombieCharBuf;
    use Carp;
    sub new { confess "ZombieCharBuf objects can only be created from C." }
    sub DESTROY { }
}

{
    package KinoSearch::Object::Err;
    sub do_to_string { shift->to_string }
    use Scalar::Util qw( blessed );
    use Carp qw( confess longmess );
    use overload
        '""'     => \&do_to_string,
        fallback => 1;

    sub new {
        my ( $either, $message ) = @_;
        my ( undef, $file, $line ) = caller;
        $message .= ", $file line $line\n";
        return $either->_new(
            mess => KinoSearch::Object::CharBuf->new($message) );
    }

    sub do_throw {
        my $err = shift;
        $err->cat_mess( longmess() );
        die $err;
    }

    our $error;
    sub set_error {
        my $val = $_[1];
        if ( defined $val ) {
            confess("Not a KinoSearch::Object::Err")
                unless ( blessed($val)
                && $val->isa("KinoSearch::Object::Err") );
        }
        $error = $val;
    }
    sub get_error {$error}
}

{
    package KinoSearch::Object::Hash;
    no warnings 'redefine';
    sub deserialize { shift->_deserialize(@_) }
}

{
    package KinoSearch::Object::VArray;
    no warnings 'redefine';
    sub clone       { CORE::shift->_clone }
    sub deserialize { CORE::shift->_deserialize(@_) }
}

{
    package KinoSearch::Store::FileHandle;
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
    package KinoSearch::Store::FSFileHandle;

    sub open {
        my ( $either, %args ) = @_;
        $args{flags} ||= 0;
        $args{flags}
            |= KinoSearch::Store::FileHandle::build_fh_flags( \%args );
        return $either->_open(%args);
    }
}

{
    package KinoSearch::Store::FSFolder;
    use File::Spec::Functions qw( rel2abs );
    sub absolutify { return rel2abs( $_[1] ) }
}

{
    package KinoSearch::Store::RAMFileHandle;

    sub open {
        my ( $either, %args ) = @_;
        $args{flags} ||= 0;
        $args{flags}
            |= KinoSearch::Store::FileHandle::build_fh_flags( \%args );
        return $either->_open(%args);
    }
}

{
    package KinoSearch::Util::Debug;
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
    package KinoSearch::Util::Json;
    use Scalar::Util qw( blessed );
    use KinoSearch qw( to_kino );

    use JSON::XS qw();

    my $json_encoder = JSON::XS->new->pretty(1)->canonical(1);

    sub slurp_json {
        my ( undef, %args ) = @_;
        my $instream = $args{folder}->open_in( $args{path} )
            or return;
        my $len = $instream->length;
        my $json;
        $instream->read( $json, $len );
        my $result = eval { to_kino( $json_encoder->decode($json) ) };
        if ( $@ or !$result ) {
            KinoSearch::Object::Err->set_error(
                KinoSearch::Object::Err->new( $@ || "Failed to decode JSON" )
            );
            return;
        }
        return $result;
    }

    sub spew_json {
        my ( undef, %args ) = @_;
        my $json = eval { $json_encoder->encode( $args{'dump'} ) };
        if ( !defined $json ) {
            KinoSearch::Object::Err->set_error(
                KinoSearch::Object::Err->new($@) );
            return 0;
        }
        my $outstream = $args{folder}->open_out( $args{path} );
        return 0 unless $outstream;
        eval {
            $outstream->print($json);
            $outstream->close;
        };
        if ($@) {
            my $error;
            if ( blessed($@) && $@->isa("KinoSearch::Object::Err") ) {
                $error = $@;
            }
            else {
                $error = KinoSearch::Object::Err->new($@);
            }
            KinoSearch::Object::Err->set_error($error);
            return 0;
        }
        return 1;
    }

    sub to_json {
        my ( undef, $dump ) = @_;
        return $json_encoder->encode($dump);
    }

    sub from_json {
        return to_kino( $json_encoder->decode( $_[1] ) );
    }
}

{
    package KinoSearch::Object::Host;
    BEGIN {
        if ( !__PACKAGE__->isa('KinoSearch::Object::Obj') ) {
            push our @ISA, 'KinoSearch::Object::Obj';
        }
    }
}

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = KinoSearch    PACKAGE = KinoSearch

BOOT:
    kino_KinoSearch_bootstrap();

IV
_dummy_function()
CODE:
    RETVAL = 1;
OUTPUT:
    RETVAL

SV*
to_kino(sv)
    SV *sv;
CODE:
{
    kino_Obj *obj = XSBind_perl_to_kino(sv);
    RETVAL = KINO_OBJ_TO_SV_NOINC(obj);
}
OUTPUT: RETVAL

SV*
to_perl(sv)
    SV *sv;
CODE:
{
    if (sv_isobject(sv) && sv_derived_from(sv, "KinoSearch::Object::Obj")) {
        IV tmp = SvIV(SvRV(sv));
        kino_Obj* obj = INT2PTR(kino_Obj*, tmp);
        RETVAL = XSBind_kino_to_perl(obj);
    }
    else {
        RETVAL = newSVsv(sv);
    }
}
OUTPUT: RETVAL
END_XS_CODE

Clownfish::Binding::Perl::Class->register(
    parcel     => "KinoSearch",
    class_name => "KinoSearch",
    xs_code    => $xs_code,
);


