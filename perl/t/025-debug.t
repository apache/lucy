# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

use strict;
use warnings;
use lib 'buildlib';

use Test::More;
use File::Spec::Functions qw( catfile );
use Fcntl;
use Lucy::Util::Debug qw(
    DEBUG_ENABLED
    DEBUG_PRINT
    DEBUG
    ASSERT
    set_env_cache
);
use Lucy::Test::TestUtils qw( working_dir );

BEGIN {
    if ( !DEBUG_ENABLED() ) {
        plan( skip_all => 'DEBUG not enabled' );
    }
    elsif ( $ENV{LUCY_VALGRIND} ) {
        plan( skip_all => 'Tests disabled under valgrind' );
    }
    else {
        plan( tests => 7 );
    }
}

my $stderr_dumpfile = catfile( working_dir(), 'lucy_garbage' );
unlink $stderr_dumpfile;
sysopen( STDERR, $stderr_dumpfile, O_CREAT | O_WRONLY | O_EXCL )
    or die "Failed to redirect STDERR";

DEBUG_PRINT("Roach Motel");
like( slurp_file($stderr_dumpfile), qr/Roach Motel/, "DEBUG_PRINT" );

ASSERT(1);
pass("ASSERT(true) didn't die");

SKIP: {
    skip( "Windows fork not supported by Lucy", 3 ) if $^O =~ /mswin/i;

    my $stderr_out = capture_debug( 'Lucy.xs', 'Borax' );
    like( $stderr_out, qr/Borax/, "DEBUG - file name" );

    $stderr_out = capture_debug( 'XS_Lucy__Util__Debug_DEBUG', "Strychnine" );
    like( $stderr_out, qr/Strychnine/, "DEBUG - function name" );

    $stderr_out = capture_debug( 'Lucy*', 'Raid' );
    like( $stderr_out, qr/Raid/, "DEBUG - wildcard" );

    my $pid = fork();
    if ( $pid == 0 ) {    # child
        ASSERT(0);
        exit;
    }
    else {
        waitpid( $pid, 0 );
        like(
            slurp_file($stderr_dumpfile),
            qr/ASSERT FAILED/,
            "failing ASSERT"
        );
    }
}

set_env_cache("");
DEBUG("Slug and Snail Death");
unlike( slurp_file($stderr_dumpfile), qr/Slug/, "DEBUG disabled by default" );

# Clean up.
unlink $stderr_dumpfile;

sub capture_debug {
    my ( $fake_env_var, $debug_string ) = @_;
    my $pid = fork();
    if ( $pid == 0 ) {    # child
        set_env_cache($fake_env_var);
        DEBUG($debug_string);
        exit;
    }
    else {
        waitpid( $pid, 0 );
    }
    return slurp_file($stderr_dumpfile);
}

sub slurp_file {
    my $path = shift;
    open( my $fh, '<', $path ) or die "Can't open '$path' for reading: $!";
    my $content = do { local $/; <$fh> };
    return $content;
}
