use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 16;
use KinoSearch::Test::TestUtils qw( test_analyzer );

my $source_text = 'Eats, shoots and leaves.';

my $case_folder = KinoSearch::Analysis::CaseFolder->new;
my $tokenizer   = KinoSearch::Analysis::Tokenizer->new;
my $stopalizer  = KinoSearch::Analysis::Stopalizer->new( language => 'en' );
my $stemmer     = KinoSearch::Analysis::Stemmer->new( language => 'en' );

my $polyanalyzer
    = KinoSearch::Analysis::PolyAnalyzer->new( analyzers => [], );
test_analyzer( $polyanalyzer, $source_text, [$source_text],
    'no sub analyzers' );

$polyanalyzer
    = KinoSearch::Analysis::PolyAnalyzer->new( analyzers => [$case_folder], );
test_analyzer(
    $polyanalyzer, $source_text,
    ['eats, shoots and leaves.'],
    'with CaseFolder'
);

$polyanalyzer = KinoSearch::Analysis::PolyAnalyzer->new(
    analyzers => [ $case_folder, $tokenizer ], );
test_analyzer(
    $polyanalyzer, $source_text,
    [ 'eats', 'shoots', 'and', 'leaves' ],
    'with Tokenizer'
);

$polyanalyzer = KinoSearch::Analysis::PolyAnalyzer->new(
    analyzers => [ $case_folder, $tokenizer, $stopalizer ], );
test_analyzer(
    $polyanalyzer, $source_text,
    [ 'eats', 'shoots', 'leaves' ],
    'with Stopalizer'
);

$polyanalyzer = KinoSearch::Analysis::PolyAnalyzer->new(
    analyzers => [ $case_folder, $tokenizer, $stopalizer, $stemmer, ], );
test_analyzer( $polyanalyzer, $source_text, [ 'eat', 'shoot', 'leav' ],
    'with Stemmer' );

ok( $polyanalyzer->get_analyzers(), "get_analyzers method" );
