use strict;
use warnings;

use Test::More tests => 1;
use Lucy::Test;

my ( $varray, $evil_twin );

$varray = Lucy::Object::VArray->new( capacity => 5 );
$varray->push( Lucy::Object::CharBuf->new($_) ) for 1 .. 5;
$varray->delete(3);

$evil_twin = $varray->_clone;
is_deeply( $evil_twin->to_perl, $varray->to_perl, "clone" );

