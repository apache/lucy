use strict;
use warnings;

use lib 'buildlib';
use KinoSearch::Test;

package MySchema;
use base qw( KinoSearch::Plan::Schema );

sub new {
    my $self = shift->SUPER::new(@_);
    my $tokenizer = KinoSearch::Analysis::Tokenizer->new( pattern => '\S+' );
    my $wordchar_tokenizer
        = KinoSearch::Analysis::Tokenizer->new( pattern => '\w+', );
    my $stopalizer
        = KinoSearch::Analysis::Stopalizer->new( stoplist => { x => 1 } );
    my $fancy_analyzer = KinoSearch::Analysis::PolyAnalyzer->new(
        analyzers => [ $wordchar_tokenizer, $stopalizer, ], );

    my $plain = KinoSearch::Plan::FullTextType->new( analyzer => $tokenizer );
    my $fancy
        = KinoSearch::Plan::FullTextType->new( analyzer => $fancy_analyzer );
    $self->spec_field( name => 'plain', type => $plain );
    $self->spec_field( name => 'fancy', type => $fancy );
    return $self;
}

package main;

# Build index.
my $doc_set = KinoSearch::Test::TestUtils::doc_set()->to_perl;
my $folder  = KinoSearch::Store::RAMFolder->new;
my $schema  = MySchema->new;
my $indexer = KinoSearch::Index::Indexer->new(
    schema => $schema,
    index  => $folder,
);
for my $content_string (@$doc_set) {
    $indexer->add_doc(
        {   plain => $content_string,
            fancy => $content_string,
        }
    );
}
$indexer->commit;

KinoSearch::Test::TestQueryParserSyntax::run_tests($folder);

