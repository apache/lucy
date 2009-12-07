use strict;
use warnings;

use Test::More tests => 5;

use Clownfish::CBlock;
use Clownfish::Parser;

my $parser = Clownfish::Parser->new;

my $block = Clownfish::CBlock->new( contents => 'int foo;' );
isa_ok( $block, "Clownfish::CBlock" );
is( $block->get_contents, 'int foo;', "get_contents" );
eval { Clownfish::CBlock->new };
like( $@, qr/contents/, "content required" );

$block = $parser->embed_c(qq| __C__\n#define FOO 1\n__END_C__  |);

isa_ok( $block, "Clownfish::CBlock" );
is( $block->get_contents, "#define FOO 1\n", "parse embed_c" );

