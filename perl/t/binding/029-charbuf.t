use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 6;
use Storable qw( freeze thaw );
use KinoSearch::Test::TestUtils qw( utf8_test_strings );

my ( $smiley, $not_a_smiley, $frowny ) = utf8_test_strings();

my $charbuf = KinoSearch::Object::CharBuf->new($smiley);
isa_ok( $charbuf, "KinoSearch::Object::CharBuf" );
is( $charbuf->to_perl, $smiley, "round trip UTF-8" );

$charbuf = KinoSearch::Object::CharBuf->new($smiley);
my $dupe = thaw( freeze($charbuf) );
isa_ok( $dupe, "KinoSearch::Object::CharBuf",
    "thaw/freeze produces correct object" );
is( $dupe->to_perl, $charbuf->to_perl, "freeze/thaw" );

my $clone = $charbuf->clone;
is( $clone->to_perl, KinoSearch::Object::CharBuf->new($smiley)->to_perl,
    "clone" );

my $ram_file = KinoSearch::Store::RAMFile->new;
my $outstream = KinoSearch::Store::OutStream->open( file => $ram_file )
    or die KinoSearch->error;
$charbuf->serialize($outstream);
$outstream->close;
my $instream = KinoSearch::Store::InStream->open( file => $ram_file )
    or die KinoSearch->error;
my $deserialized = KinoSearch::Object::CharBuf->deserialize($instream);
is_deeply( $charbuf->to_perl, $deserialized->to_perl,
    "serialize/deserialize" );

