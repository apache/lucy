use strict;
use warnings;

use Test::More tests => 2;

use Boilerplater::Parser;

my $parser = Boilerplater::Parser->new;
$parser->parcel_definition('parcel Boil;')
    or die "failed to process parcel_definition";

my %args = (
    return_type => $parser->type('Obj*'),
    class_name  => 'Boil::Foo',
    class_cnick => 'Foo',
    param_list  => $parser->param_list('(Foo* self)'),
    macro_sym   => 'Return_An_Obj',
    parcel      => 'Boil',
);

my $not_final_method = Boilerplater::Method->new(%args);
my $final_method     = $not_final_method->finalize;
ok( !$not_final_method->final, "not final by default" );
ok( $final_method->final,      "finalize" );

