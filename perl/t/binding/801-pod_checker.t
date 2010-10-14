use strict;
use warnings;

use Test::More 'no_plan';
use Pod::Checker;

use File::Find qw( find );

my @filepaths;
find(
    {   no_chdir => 1,
        wanted   => sub {
            return unless $File::Find::name =~ /\.(pm|pod)$/;
            push @filepaths, $File::Find::name;
            }
    },
    'lib'
);

for my $path (@filepaths) {
    my $pod_ok = podchecker( $path, undef, -warnings => 0 );
    if ( $pod_ok == -1 ) {
        # No POD.
    }
    elsif ( $pod_ok == 0 ) {
        pass("POD ok for '$path'");
    }
    else {
        fail("Bad POD for '$path'");
    }
}
