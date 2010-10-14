use strict;
use warnings;

use Test::More tests => 1;
use Storable qw( freeze thaw );
use KinoSearch::Test;

my $orig   = KinoSearch::Object::ByteBuf->new("foo");
my $frozen = freeze($orig);
my $thawed = thaw($frozen);
is( $thawed->to_perl, $orig->to_perl, "freeze/thaw" );

