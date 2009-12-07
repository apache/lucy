use strict;
use warnings;

use Test::More tests => 10;

BEGIN { use_ok('Boilerplater::DocuComment') }
use Boilerplater::Parser;

my $parser = Boilerplater::Parser->new;
isa_ok( $parser->docucomment('/** foo. */'), "Boilerplater::DocuComment" );

my $text = <<'END_COMMENT';
/** 
 * Brief description.  Long description. 
 * 
 * More long description.
 * 
 * @param foo A foo.
 * @param bar A bar.
 *
 * @param baz A baz.
 * @return a return value.
 */
END_COMMENT

my $docucomment = Boilerplater::DocuComment->parse($text);

like(
    $docucomment->get_description,
    qr/^Brief.*long description.\s*\Z/ims,
    "get_description"
);
is( $docucomment->get_brief, "Brief description.", "brief" );
like( $docucomment->get_long, qr/^Long.*long description.\s*\Z/ims, "long" );
is_deeply( $docucomment->get_param_names, [qw( foo bar baz )],
    "param names" );
is( $docucomment->get_param_docs->[0], "A foo.", '@param terminated by @' );
is( $docucomment->get_param_docs->[1],
    "A bar.", '@param terminated by empty line' );
is( $docucomment->get_param_docs->[2],
    "A baz.", '@param terminated next element, @return' );
is( $docucomment->get_retval, "a return value.", "get_retval" );

