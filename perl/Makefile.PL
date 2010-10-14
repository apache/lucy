use strict;
use warnings;

print <<EOSTUFF;

This distribution uses Module::Build.  To install it, enter the following
commands:

    perl Build.PL
    perl Build
    perl Build test
    perl Build install

EOSTUFF

exit(1);
