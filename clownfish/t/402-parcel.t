use strict;
use warnings;

use Test::More tests => 7;

BEGIN { use_ok('Clownfish::Parcel') }

package ClownfishyThing;
use base qw( Clownfish::Symbol );

sub new {
    return shift->SUPER::new( micro_sym => 'sym', exposure => 'parcel', @_ );
}

package main;

# Register singleton.
Clownfish::Parcel->singleton( name => 'Crustacean', cnick => 'Crust', );

my $thing = ClownfishyThing->new;
is( $thing->get_prefix, '', 'get_prefix with no parcel' );
is( $thing->get_Prefix, '', 'get_Prefix with no parcel' );
is( $thing->get_PREFIX, '', 'get_PREFIx with no parcel' );

$thing = ClownfishyThing->new( parcel => 'Crustacean' );
is( $thing->get_prefix, 'crust_', 'get_prefix with parcel' );
is( $thing->get_Prefix, 'Crust_', 'get_Prefix with parcel' );
is( $thing->get_PREFIX, 'CRUST_', 'get_PREFIx with parcel' );

