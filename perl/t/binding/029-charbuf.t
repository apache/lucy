use strict;
use warnings;
use lib 'buildlib';

use Lucy::Test;
use Test::More tests => 3;

my $smiley = "\x{263a}";

my $charbuf = Lucy::Object::CharBuf->new($smiley);
isa_ok( $charbuf, "Lucy::Object::CharBuf" );
is( $charbuf->to_perl, $smiley, "round trip UTF-8" );

my $clone = $charbuf->clone;
is( $clone->to_perl, Lucy::Object::CharBuf->new($smiley)->to_perl,
    "clone" );

