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

package Lucy::Test::TestUtils;
use base qw( Exporter );

our $VERSION = '0.003002';
$VERSION = eval $VERSION;

our @EXPORT_OK = qw(
    working_dir
    create_working_dir
    remove_working_dir
    create_index
    create_uscon_index
    test_index_loc
    persistent_test_index_loc
    init_test_index_loc
    get_uscon_docs
    utf8_test_strings
    test_analyzer
    doc_ids_from_td_coll
    modulo_set
);

use Lucy;
use Lucy::Test;
use File::Spec::Functions qw( catdir catfile curdir );
use Encode qw( _utf8_off );
use File::Path qw( rmtree );
use Carp;

my $working_dir = catfile( curdir(), 'lucy_test' );

# Return a directory within the system's temp directory where we will put all
# testing scratch files.
sub working_dir {$working_dir}

sub create_working_dir {
    mkdir( $working_dir, 0700 ) or die "Can't mkdir '$working_dir': $!";
}

# Verify that this user owns the working dir, then zap it.  Returns true upon
# success.
sub remove_working_dir {
    return unless -d $working_dir;
    rmtree $working_dir;
    return 1;
}

# Return a location for a test index to be used by a single test file.  If
# the test file crashes it cannot clean up after itself, so we put the cleanup
# routine in a single test file to be run at or near the end of the test
# suite.
sub test_index_loc {
    return catdir( $working_dir, 'test_index' );
}

# Return a location for a test index intended to be shared by multiple test
# files.  It will be cleaned as above.
sub persistent_test_index_loc {
    return catdir( $working_dir, 'persistent_test_index' );
}

# Destroy anything left over in the test_index location, then create the
# directory.  Finally, return the path.
sub init_test_index_loc {
    my $dir = test_index_loc();
    rmtree $dir;
    die "Can't clean up '$dir'" if -e $dir;
    mkdir $dir or die "Can't mkdir '$dir': $!";
    return $dir;
}

# Build a RAM index, using the supplied array of strings as source material.
# The index will have a single field: "content".
sub create_index {
    my $folder  = Lucy::Store::RAMFolder->new;
    my $indexer = Lucy::Index::Indexer->new(
        index  => $folder,
        schema => Lucy::Test::TestSchema->new,
    );
    $indexer->add_doc( { content => $_ } ) for @_;
    $indexer->commit;
    return $folder;
}

# Slurp us constitition docs and build hashrefs.
sub get_uscon_docs {

    my $uscon_dir = catdir( 'sample', 'us_constitution' );
    opendir( my $uscon_dh, $uscon_dir )
        or die "couldn't opendir '$uscon_dir': $!";
    my @filenames = grep {/\.txt$/} sort readdir $uscon_dh;
    closedir $uscon_dh or die "couldn't closedir '$uscon_dir': $!";

    my %docs;

    for my $filename (@filenames) {
        my $filepath = catfile( $uscon_dir, $filename );
        open( my $fh, '<', $filepath )
            or die "couldn't open file '$filepath': $!";
        my $content = do { local $/; <$fh> };
        $content =~ /\A(.+?)^\s+(.*)/ms
            or die "Can't extract title/bodytext from '$filepath'";
        my $title    = $1;
        my $bodytext = $2;
        $bodytext =~ s/\s+/ /sg;
        my $category
            = $filename =~ /art/      ? 'article'
            : $filename =~ /amend/    ? 'amendment'
            : $filename =~ /preamble/ ? 'preamble'
            :   confess "Can't derive category for $filename";

        $docs{$filename} = {
            title    => $title,
            bodytext => $bodytext,
            url      => "/us_constitution/$filename",
            category => $category,
        };
    }

    return \%docs;
}

sub _uscon_schema {
    my $schema     = Lucy::Plan::Schema->new;
    my $analyzer   = Lucy::Analysis::EasyAnalyzer->new( language => 'en' );
    my $title_type = Lucy::Plan::FullTextType->new( analyzer => $analyzer, );
    my $content_type = Lucy::Plan::FullTextType->new(
        analyzer      => $analyzer,
        highlightable => 1,
    );
    my $url_type = Lucy::Plan::StringType->new( indexed => 0, );
    my $cat_type = Lucy::Plan::StringType->new;
    $schema->spec_field( name => 'title',    type => $title_type );
    $schema->spec_field( name => 'content',  type => $content_type );
    $schema->spec_field( name => 'url',      type => $url_type );
    $schema->spec_field( name => 'category', type => $cat_type );
    return $schema;
}

sub create_uscon_index {
    my $folder
        = Lucy::Store::FSFolder->new( path => persistent_test_index_loc() );
    my $indexer = Lucy::Index::Indexer->new(
        schema   => _uscon_schema(),
        index    => $folder,
        truncate => 1,
        create   => 1,
    );

    $indexer->add_doc( { content => "zz$_" } ) for ( 0 .. 10000 );
    $indexer->commit;
    undef $indexer;

    $indexer = Lucy::Index::Indexer->new( index => $folder );
    my $source_docs = get_uscon_docs();
    $indexer->add_doc( { content => $_->{bodytext} } )
        for values %$source_docs;
    $indexer->commit;
    undef $indexer;

    $indexer = Lucy::Index::Indexer->new( index => $folder );
    my @chars = ( 'a' .. 'z' );
    for ( 0 .. 1000 ) {
        my $content = '';
        for my $num_words ( 1 .. int( rand(20) ) ) {
            for ( 1 .. ( int( rand(10) ) + 10 ) ) {
                $content .= @chars[ rand(@chars) ];
            }
            $content .= ' ';
        }
        $indexer->add_doc( { content => $content } );
    }
    $indexer->optimize;
    $indexer->commit;
}

# Return 3 strings useful for verifying UTF-8 integrity.
sub utf8_test_strings {
    my $smiley       = "\x{263a}";
    my $not_a_smiley = $smiley;
    _utf8_off($not_a_smiley);
    my $frowny = $not_a_smiley;
    utf8::upgrade($frowny);
    return ( $smiley, $not_a_smiley, $frowny );
}

# Verify an Analyzer's transform, transform_text, and split methods.
sub test_analyzer {
    my ( $analyzer, $source, $expected, $message ) = @_;

    my $inversion = Lucy::Analysis::Inversion->new( text => $source );
    $inversion = $analyzer->transform($inversion);
    my @got;
    while ( my $token = $inversion->next ) {
        push @got, $token->get_text;
    }
    Test::More::is_deeply( \@got, $expected, "analyze: $message" );

    $inversion = $analyzer->transform_text($source);
    @got       = ();
    while ( my $token = $inversion->next ) {
        push @got, $token->get_text;
    }
    Test::More::is_deeply( \@got, $expected, "transform_text: $message" );

    @got = @{ $analyzer->split($source) };
    Test::More::is_deeply( \@got, $expected, "split: $message" );
}

# Extract all doc nums from a SortCollector.  Return two sorted array refs:
# by_score and by_id.
sub doc_ids_from_td_coll {
    my $collector = shift;
    my @by_score;
    my $match_docs = $collector->pop_match_docs;
    my @by_score_then_id = map { $_->get_doc_id }
        sort {
               $b->get_score <=> $a->get_score
            || $a->get_doc_id <=> $b->get_doc_id
        } @$match_docs;
    my @by_id = sort { $a <=> $b } @by_score_then_id;
    return ( \@by_score_then_id, \@by_id );
}

# Use a modulus to generate a set of numbers.
sub modulo_set {
    my ( $interval, $max ) = @_;
    my @out;
    for ( my $doc = $interval; $doc < $max; $doc += $interval ) {
        push @out, $doc;
    }
    return \@out;
}

1;

__END__


