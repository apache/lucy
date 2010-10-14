use strict;
use warnings;

use Test::More tests => 5;
use Clownfish::Type::VAList;
use Clownfish::Parser;

my $va_list_type = Clownfish::Type::VAList->new;
is( $va_list_type->get_specifier,
    "va_list", "specifier defaults to 'va_list'" );
is( $va_list_type->to_c, "va_list", "to_c" );

my $parser = Clownfish::Parser->new;

is( $parser->va_list_type_specifier('va_list'),
    'va_list', 'va_list_type_specifier' );
isa_ok( $parser->va_list_type('va_list'), "Clownfish::Type::VAList" );
ok( !$parser->va_list_type_specifier('va_listable'),
    "va_list_type_specifier guards against partial word matches"
);

