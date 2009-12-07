use strict;
use warnings;

use Test::More tests => 5;

use Boilerplater::CBlock;
use Boilerplater::Parser;

my $parser = Boilerplater::Parser->new;

my $block = Boilerplater::CBlock->new( contents => 'int foo;' );
isa_ok( $block, "Boilerplater::CBlock" );
is( $block->get_contents, 'int foo;', "get_contents" );
eval { Boilerplater::CBlock->new };
like( $@, qr/contents/, "content required" );

$block = $parser->embed_c(qq| __C__\n#define FOO 1\n__END_C__  |);

isa_ok( $block, "Boilerplater::CBlock" );
is( $block->get_contents, "#define FOO 1\n", "parse embed_c" );

