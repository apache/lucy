use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 6;
use KinoSearch::Test::TestUtils qw( test_analyzer );

my $stopalizer = KinoSearch::Analysis::Stopalizer->new( language => 'en' );
test_analyzer( $stopalizer, 'the', [], "single stopword stopalized" );

my $tokenizer    = KinoSearch::Analysis::Tokenizer->new;
my $polyanalyzer = KinoSearch::Analysis::PolyAnalyzer->new(
    analyzers => [ $tokenizer, $stopalizer ], );
test_analyzer( $polyanalyzer, 'i am the walrus',
    ['walrus'], "multiple stopwords stopalized" );
