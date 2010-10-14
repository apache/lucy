use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 4;
use File::Spec::Functions qw( catfile );
use File::Find qw( find );
use KinoSearch::Test::TestUtils qw(
    working_dir
    create_working_dir
    remove_working_dir
    create_uscon_index
    persistent_test_index_loc
);

remove_working_dir();
ok( !-e working_dir(), "Working dir doesn't exist" );
create_working_dir();
ok( -e working_dir(), "Working dir successfully created" );

create_uscon_index();

my $path = persistent_test_index_loc();
ok( -d $path, "created index directory" );
my $num_cfmeta = 0;
find(
    {   no_chdir => 1,
        wanted   => sub { $num_cfmeta++ if $File::Find::name =~ /cfmeta/ },
    },
    $path
);
cmp_ok( $num_cfmeta, '>', 0, "at least one .cf file exists" );

