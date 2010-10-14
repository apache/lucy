use strict;
use warnings;

use Test::More tests => 1;

use Clownfish::Method;
use Clownfish::Parser;

my $parser = Clownfish::Parser->new;
$parser->parcel_definition('parcel Neato;')
    or die "failed to process parcel_definition";

my %args = (
    return_type => $parser->type('Obj*'),
    class_name  => 'Neato::Foo',
    class_cnick => 'Foo',
    param_list  => $parser->param_list('(Foo *self)'),
    macro_sym   => 'Return_An_Obj',
    parcel      => 'Neato',
);

my $orig      = Clownfish::Method->new(%args);
my $overrider = Clownfish::Method->new(
    %args,
    param_list  => $parser->param_list('(FooJr *self)'),
    class_name  => 'Neato::Foo::FooJr',
    class_cnick => 'FooJr'
);
$overrider->override($orig);
ok( !$overrider->novel );

