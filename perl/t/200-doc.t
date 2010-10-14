use strict;
use warnings;

use Test::More tests => 10;
use Storable qw( nfreeze thaw );
use KinoSearch::Test;

my $doc = KinoSearch::Document::Doc->new;
is_deeply( $doc->get_fields, {}, "get_fields" );
is( $doc->get_doc_id, 0, "default doc_id of 0" );

$doc->{foo} = "blah";
is_deeply( $doc->get_fields, { foo => 'blah' }, "overloading" );

my %hash = ( foo => 'foo' );
$doc = KinoSearch::Document::Doc->new(
    fields => \%hash,
    doc_id => 30,
);
$hash{bar} = "blah";
is_deeply(
    $doc->get_fields,
    { foo => 'foo', bar => 'blah' },
    "using supplied hash"
);
is( $doc->get_doc_id, 30, "doc_id param" );
$doc->set_doc_id(20);
is( $doc->get_doc_id, 20, "doc_id param" );

my $frozen = nfreeze($doc);
my $thawed = thaw($frozen);
is_deeply( $thawed->get_fields, $doc->get_fields,
    "fields survive freeze/thaw" );
is( $thawed->get_doc_id, $doc->get_doc_id, "doc_id survives freeze/thaw" );
ok( $doc->equals($thawed), "equals" );

my $dump   = $doc->dump;
my $loaded = $doc->load($dump);
ok( $doc->equals($loaded), "dump => load round trip" );
