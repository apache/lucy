use strict;
use warnings;

use Test::More tests => 1;

use Boilerplater::Method;
use Boilerplater::Parser;

my $parser = Boilerplater::Parser->new;
$parser->parcel_definition('parcel Boil;')
    or die "failed to process parcel_definition";

my %args = (
    return_type => $parser->type('Obj*'),
    class_name  => 'Boil::Foo',
    class_cnick => 'Foo',
    param_list  => $parser->param_list('(Foo *self)'),
    macro_sym   => 'Return_An_Obj',
    parcel      => 'Boil',
);

my $orig      = Boilerplater::Method->new(%args);
my $overrider = Boilerplater::Method->new(
    %args,
    param_list  => $parser->param_list('(FooJr *self)'),
    class_name  => 'Boil::Foo::FooJr',
    class_cnick => 'FooJr'
);
$overrider->override($orig);
ok( !$overrider->novel );

