use strict;
use warnings;

use Test::More tests => 1;

use File::Spec::Functions qw( curdir );
use Env qw( @PATH );

unshift @PATH, curdir();

# capture and parse output of 'charm_test' 
my $charm_test_output = qx|charm_test|;
$charm_test_output =~ /TOTAL FAILED:\s*(\d+)/ 
    or die "Didn't receive expected output from 'charm_test'";
my $total_failed = $1;

print STDERR "\n$charm_test_output\n";

is( $total_failed, 0, "No failures in Charmonizer's test suite" );


