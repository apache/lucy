use strict;
use warnings;

use Test::More tests => 8;
use Storable qw( nfreeze thaw );
use KinoSearch::Test;

my $doc = KinoSearch::Document::HitDoc->new;
is( $doc->get_doc_id, 0,   "default doc_id of 0" );
is( $doc->get_score,  0.0, "default score of 0.0" );
$doc->set_score(2);
is( $doc->get_score, 2, "set_score" );

$doc->{foo} = "foo foo";
is( $doc->{foo}, "foo foo", "hash overloading" );

my $frozen = nfreeze($doc);
my $thawed = thaw($frozen);
is( ref($thawed),       ref($doc),       "correct class after freeze/thaw" );
is( $thawed->get_score, $doc->get_score, "score survives freeze/thaw" );
ok( $doc->equals($thawed), "equals" );

my $dump   = $doc->dump;
my $loaded = $doc->load($dump);
ok( $doc->equals($loaded), "dump => load round trip" );
