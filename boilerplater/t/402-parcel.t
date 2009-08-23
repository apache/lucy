use strict;
use warnings;

use Test::More tests => 7;

BEGIN { use_ok('Boilerplater::Parcel') }

package BoilingThing;
use base qw( Boilerplater::Symbol );

sub new {
    return shift->SUPER::new( micro_sym => 'sym', exposure => 'parcel', @_ );
}

package main;

# Register singleton.
Boilerplater::Parcel->singleton( name => 'Boil' );

my $thing = BoilingThing->new;
is( $thing->get_prefix, '', 'get_prefix with no parcel' );
is( $thing->get_Prefix, '', 'get_Prefix with no parcel' );
is( $thing->get_PREFIX, '', 'get_PREFIx with no parcel' );

$thing = BoilingThing->new( parcel => 'Boil' );
is( $thing->get_prefix, 'boil_', 'get_prefix with parcel' );
is( $thing->get_Prefix, 'Boil_', 'get_Prefix with parcel' );
is( $thing->get_PREFIX, 'BOIL_', 'get_PREFIx with parcel' );

