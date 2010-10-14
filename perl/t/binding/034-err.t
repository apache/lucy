use strict;
use warnings;

use Test::More tests => 1;
use KinoSearch::Test;

my $err = KinoSearch::Object::Err->new("Bad stuff happened");

isa_ok( $err, 'KinoSearch::Object::Err', "new" );

