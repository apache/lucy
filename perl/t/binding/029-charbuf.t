use strict;
use warnings;
use lib 'buildlib';

use Lucy::Test;
use Test::More tests => 6;
use Storable qw( freeze thaw );

my $smiley = "\x{263a}";

my $charbuf = Lucy::Object::CharBuf->new($smiley);
isa_ok( $charbuf, "Lucy::Object::CharBuf" );
is( $charbuf->to_perl, $smiley, "round trip UTF-8" );

$charbuf = Lucy::Object::CharBuf->new($smiley);
my $dupe = thaw( freeze($charbuf) );
isa_ok( $dupe, "Lucy::Object::CharBuf",
    "thaw/freeze produces correct object" );
is( $dupe->to_perl, $charbuf->to_perl, "freeze/thaw" );

my $clone = $charbuf->clone;
is( $clone->to_perl, Lucy::Object::CharBuf->new($smiley)->to_perl,
    "clone" );

my $ram_file = Lucy::Store::RAMFile->new;
my $outstream = Lucy::Store::OutStream->open( file => $ram_file )
    or die Lucy->error;
$charbuf->serialize($outstream);
$outstream->close;
my $instream = Lucy::Store::InStream->open( file => $ram_file )
    or die Lucy->error;
my $deserialized = Lucy::Object::CharBuf->deserialize($instream);
is_deeply( $charbuf->to_perl, $deserialized->to_perl,
    "serialize/deserialize" );

