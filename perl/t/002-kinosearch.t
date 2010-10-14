use strict;
use warnings;

use Test::More;
use File::Find 'find';

my @modules;

my %excluded = map { ( $_ => 1 ) } qw(
    KinoSearch::Analysis::LCNormalizer
    KinoSearch::Index::Term
    KinoSearch::InvIndex
    KinoSearch::InvIndexer
    KinoSearch::QueryParser::QueryParser
    KinoSearch::Search::BooleanQuery
    KinoSearch::Search::Scorer
    KinoSearch::Search::SearchClient
    KinoSearch::Search::SearchServer
);

find(
    {   no_chdir => 1,
        wanted   => sub {
            return unless $File::Find::name =~ /\.pm$/;
            push @modules, $File::Find::name;
            }
    },
    'lib'
);

plan( tests => scalar @modules );

for (@modules) {
    s/^.*?KinoSearch/KinoSearch/;
    s/^.*?KSx/KSx/;
    s/\.pm$//;
    s/\W+/::/g;
    if ( $excluded{$_} ) {
        eval qq|use $_;|;
        like( $@, qr/removed|replaced|renamed/i,
            "Removed module '$_' throws error on load" );
    }
    else {
        use_ok($_);
    }
}

