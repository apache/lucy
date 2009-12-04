use strict;
use warnings;

use Test::More tests => 3;
use Storable qw( nfreeze thaw );
use Lucy::Test;

my ( $varray, $evil_twin );

$varray = Lucy::Object::VArray->new( capacity => 5 );
$varray->push( Lucy::Object::CharBuf->new($_) ) for 1 .. 5;
$varray->delete(3);
my $frozen = nfreeze($varray);
my $thawed = thaw($frozen);
is_deeply( $thawed->to_perl, $varray->to_perl, "freeze/thaw" );

my $ram_file = Lucy::Store::RAMFile->new;
my $outstream = Lucy::Store::OutStream->open( file => $ram_file )
    or die Lucy->error;
$varray->serialize($outstream);
$outstream->close;
my $instream = Lucy::Store::InStream->open( file => $ram_file )
    or die Lucy->error;
my $deserialized = $varray->deserialize($instream);
is_deeply( $varray->to_perl, $deserialized->to_perl,
    "serialize/deserialize" );

$evil_twin = $varray->_clone;
is_deeply( $evil_twin->to_perl, $varray->to_perl, "clone" );

