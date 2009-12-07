use strict;
use warnings;

use Test::More tests => 5;
use Boilerplater::Type::VAList;
use Boilerplater::Parser;

my $va_list_type = Boilerplater::Type::VAList->new;
is( $va_list_type->get_specifier,
    "va_list", "specifier defaults to 'va_list'" );
is( $va_list_type->to_c, "va_list", "to_c" );

my $parser = Boilerplater::Parser->new;

is( $parser->va_list_type_specifier('va_list'),
    'va_list', 'va_list_type_specifier' );
isa_ok( $parser->va_list_type('va_list'), "Boilerplater::Type::VAList" );
ok( !$parser->va_list_type_specifier('va_listable'),
    "va_list_type_specifier guards against partial word matches"
);

