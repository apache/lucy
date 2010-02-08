use strict;
use warnings;

use Test::More tests => 4;
use Storable qw( nfreeze thaw );
use Lucy::Test;
use Lucy::Util::ToolSet qw( to_perl to_lucy );

my $hash = Lucy::Object::Hash->new( capacity => 10 );
$hash->store( "foo", Lucy::Object::CharBuf->new("bar") );
$hash->store( "baz", Lucy::Object::CharBuf->new("banana") );

ok( !defined( $hash->fetch("blah") ),
    "fetch for a non-existent key returns undef" );

my $frozen = nfreeze($hash);
my $thawed = thaw($frozen);
is_deeply( $thawed->to_perl, $hash->to_perl, "freeze/thaw" );

my $ram_file = Lucy::Store::RAMFile->new;
my $outstream = Lucy::Store::OutStream->open( file => $ram_file )
    or die Lucy->error;
$hash->serialize($outstream);
$outstream->close;
my $instream = Lucy::Store::InStream->open( file => $ram_file )
    or die Lucy->error;
my $deserialized = $hash->deserialize($instream);
is_deeply( $hash->to_perl, $deserialized->to_perl, "serialize/deserialize" );

my %hash_with_utf8_keys = ( "\x{263a}" => "foo" );
my $round_tripped = to_perl( to_lucy( \%hash_with_utf8_keys ) );
is_deeply( $round_tripped, \%hash_with_utf8_keys,
    "Round trip conversion of hash with UTF-8 keys" );
