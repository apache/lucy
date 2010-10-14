use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 5;
use KinoSearch::Test::TestUtils qw( utf8_test_strings test_analyzer );

package TestAnalyzer;
use base qw( KinoSearch::Analysis::Analyzer );
sub transform { $_[1] }    # satisfy mandatory override

package main;
my $analyzer = TestAnalyzer->new;

my ( $smiley, $not_a_smiley, $frowny ) = utf8_test_strings();

my $got = $analyzer->split($not_a_smiley)->[0];
is( $got, $frowny, "split() upgrades non-UTF-8 correctly" );

$got = $analyzer->split($smiley)->[0];
is( $got, $smiley, "split() handles UTF-8 correctly" );

test_analyzer( $analyzer, 'foo', ['foo'], "Analyzer (no-op)" );
