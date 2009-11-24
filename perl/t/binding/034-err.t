use strict;
use warnings;

use Test::More tests => 1;
use Lucy::Test;

my $err = Lucy::Object::Err->new("Bad stuff happened");

isa_ok( $err, 'Lucy::Object::Err', "new" );

