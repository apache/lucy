use strict;
use warnings;

package KinoSearch::Redacted;
use Exporter;
BEGIN {
    our @ISA       = qw( Exporter );
    our @EXPORT_OK = qw( list );
}

# Return a partial list of KinoSearch classes which were once public but are
# now either deprecated, removed, or moved.

sub redacted {
    return qw(
        KinoSearch::Analysis::LCNormalizer
        KinoSearch::Analysis::Token
        KinoSearch::Analysis::TokenBatch
        KinoSearch::Index::Term
        KinoSearch::InvIndex
        KinoSearch::InvIndexer
        KinoSearch::QueryParser::QueryParser
        KinoSearch::Search::BooleanQuery
        KinoSearch::Search::QueryFilter
        KinoSearch::Search::SearchServer
        KinoSearch::Search::SearchClient
    );
}

# Hide additional stuff from PAUSE and search.cpan.org.
sub hidden {
    return qw(
        KinoSearch::Analysis::Inversion
        KinoSearch::Object::Num
        KinoSearch::Plan::Int32Type
        KinoSearch::Plan::Int64Type
        KinoSearch::Plan::Float32Type
        KinoSearch::Plan::Float64Type
        KinoSearch::Redacted
        KinoSearch::Test::Object::TestCharBuf
        KinoSearch::Test::TestUtils
        KinoSearch::Test::USConSchema
        KinoSearch::Util::BitVector
    );
}

1;
