use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 1;
use KinoSearch::Test::TestUtils qw( remove_working_dir working_dir );

remove_working_dir();
ok( !-e working_dir(), "working_dir is no more" );

