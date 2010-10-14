use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 3;
use KinoSearch::Test::TestUtils qw( test_analyzer );

my $case_folder = KinoSearch::Analysis::CaseFolder->new;

test_analyzer( $case_folder, "caPiTal ofFensE",
    ['capital offense'], 'lc plain text' );
