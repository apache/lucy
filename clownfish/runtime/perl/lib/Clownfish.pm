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
use Exporter;

our $VERSION = '0.003000';
$VERSION = eval $VERSION;

# On most UNIX variants, this flag makes DynaLoader pass RTLD_GLOBAL to
# dl_open, so extensions can resolve the needed symbols without explicitly
# linking against the DSO.
sub dl_load_flags { 1 }

BEGIN {
    require DynaLoader;
    our @ISA = qw( DynaLoader Exporter );
    # This loads a large number of disparate subs.
    bootstrap Lucy '0.3.0';
    _init_autobindings();
    our @EXPORT_OK = qw( to_clownfish to_perl kdump );
}

sub kdump {
    require Data::Dumper;
    my $kdumper = Data::Dumper->new( [@_] );
    $kdumper->Sortkeys( sub { return [ sort keys %{ $_[0] } ] } );
    $kdumper->Indent(1);
    warn $kdumper->Dump;
}

sub error {$Clownfish::Err::error}

{
    package Lucy::Util::IndexFileNames;
    our $VERSION = '0.003000';
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
    package Clownfish::Util::StringHelper;
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;
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
    package Lucy::Object::Obj;
    BEGIN { our @ISA = qw( Clownfish::Obj ) }
}

{
    package Lucy::Analysis::RegexTokenizer;
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;

    sub compile_token_re { return qr/$_[1]/ }

    sub new {
        my ( $either, %args ) = @_;
        my $token_re = delete $args{token_re};
        $args{pattern} = "$token_re" if $token_re;
        return $either->_new(%args);
    }
}

{
    package Lucy::Document::Doc;
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;
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
    package Clownfish::LockFreeRegistry;
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;
    no warnings 'redefine';
    sub DESTROY { }    # leak all
}

{
    package Clownfish::Obj;
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;
    use Lucy qw( to_clownfish to_perl );
    sub load { return $_[0]->_load( to_clownfish( $_[1] ) ) }
}

{
    package Clownfish::VTable;
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;

    sub find_parent_class {
        my ( undef, $package ) = @_;
        no strict 'refs';
        for my $parent ( @{"$package\::ISA"} ) {
            return $parent if $parent->isa('Clownfish::Obj');
        }
        return;
    }

    sub fresh_host_methods {
        my ( undef, $package ) = @_;
        no strict 'refs';
        my $stash = \%{"$package\::"};
        my $methods
            = Clownfish::VArray->new( capacity => scalar keys %$stash );
        while ( my ( $symbol, $glob ) = each %$stash ) {
            next if ref $glob;
            next unless *$glob{CODE};
            $methods->push( Clownfish::CharBuf->new($symbol) );
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

    no warnings 'redefine';
    sub DESTROY { }    # leak all
}

{
    package Lucy::Index::Indexer;
    our $VERSION = '0.003000';
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
    our $VERSION = '0.003000';
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
    package Lucy::Index::Segment;
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;
    use Lucy qw( to_clownfish );
    sub store_metadata {
        my ( $self, %args ) = @_;
        $self->_store_metadata( %args,
            metadata => to_clownfish( $args{metadata} ) );
    }
}

{
    package Lucy::Search::Compiler;
    our $VERSION = '0.003000';
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
    package Lucy::Search::Query;
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;

    sub make_compiler {
        my ( $self, %args ) = @_;
        $args{boost} = $self->get_boost unless defined $args{boost};
        return $self->_make_compiler(%args);
    }
}

{
    package Lucy::Search::SortRule;
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;
    use Carp;

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
    package Lucy::Object::BitVector;
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;
    sub to_arrayref { shift->to_array->to_arrayref }
}

{
    package Clownfish::ByteBuf;
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;
    {
        # Override autogenerated deserialize binding.
        no warnings 'redefine';
        sub deserialize { shift->_deserialize(@_) }
    }
}

{
    package Clownfish::ViewByteBuf;
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;
    use Carp;
    sub new { confess "ViewByteBuf objects can only be created from C." }
}

{
    package Clownfish::CharBuf;
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;

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
    package Clownfish::ViewCharBuf;
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;
    use Carp;
    sub new { confess "ViewCharBuf has no public constructor." }
}

{
    package Clownfish::ZombieCharBuf;
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;
    use Carp;
    sub new { confess "ZombieCharBuf objects can only be created from C." }
    no warnings 'redefine';
    sub DESTROY { }
}

{
    package Clownfish::Err;
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;
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
        return $either->_new( mess => Clownfish::CharBuf->new($message) );
    }

    sub do_throw {
        my $err      = shift;
        my $longmess = longmess();
        $longmess =~ s/^\s*/\t/;
        $err->cat_mess($longmess);
        die $err;
    }

    our $error;
    sub set_error {
        my $val = $_[1];
        if ( defined $val ) {
            confess("Not a Clownfish::Err")
                unless ( blessed($val)
                && $val->isa("Clownfish::Err") );
        }
        $error = $val;
    }
    sub get_error {$error}
}

{
    package Clownfish::Hash;
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;
    no warnings 'redefine';
    sub deserialize { shift->_deserialize(@_) }
}

{
    package Clownfish::VArray;
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;
    no warnings 'redefine';
    sub clone       { CORE::shift->_clone }
    sub deserialize { CORE::shift->_deserialize(@_) }
}

{
    package Lucy::Store::FileHandle;
    our $VERSION = '0.003000';
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
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;

    sub open {
        my ( $either, %args ) = @_;
        $args{flags} ||= 0;
        $args{flags} |= Lucy::Store::FileHandle::build_fh_flags( \%args );
        return $either->_open(%args);
    }
}

{
    package Lucy::Store::FSFolder;
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;
    use File::Spec::Functions qw( rel2abs );
    sub absolutify { return rel2abs( $_[1] ) }
}

{
    package Lucy::Store::RAMFileHandle;
    our $VERSION = '0.003000';
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
    our $VERSION = '0.003000';
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
    package Clownfish::Host;
    our $VERSION = '0.003000';
    $VERSION = eval $VERSION;
    BEGIN {
        if ( !__PACKAGE__->isa('Clownfish::Obj') ) {
            push our @ISA, 'Clownfish::Obj';
        }
    }
}

1;

__END__


