use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 9;
use KinoSearch::Test::TestUtils qw( test_analyzer );

my $stemmer = KinoSearch::Analysis::Stemmer->new( language => 'en' );
test_analyzer( $stemmer, 'ponies', ['poni'], "single word stemmed" );
test_analyzer( $stemmer, 'pony',   ['poni'], "stem, not just truncate" );

my $tokenizer    = KinoSearch::Analysis::Tokenizer->new;
my $polyanalyzer = KinoSearch::Analysis::PolyAnalyzer->new(
    analyzers => [ $tokenizer, $stemmer ], );
test_analyzer(
    $polyanalyzer,
    'peas porridge hot',
    [ 'pea', 'porridg', 'hot' ],
    "multiple words stemmed",
);
