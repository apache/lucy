use strict;
use warnings;

use Test::More 'no_plan';
use File::Find 'find';

my @modules;

find(
    {   no_chdir => 1,
        wanted   => sub {
            return unless $File::Find::name =~ /\.pm$/;
            push @modules, $File::Find::name;
            }
    },
    'lib'
);
for (@modules) {
    s/^.*?Clownfish/Clownfish/;
    s/\.pm$//;
    s/\W+/::/g;
    use_ok($_);
}

