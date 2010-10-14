use strict;
use warnings;
use lib 'buildlib';

use Test::More;
use File::Spec::Functions qw( catfile );
use Fcntl;
use KinoSearch::Util::Debug qw(
    DEBUG_ENABLED
    DEBUG_PRINT
    DEBUG
    ASSERT
    set_env_cache
);
use KinoSearch::Test::TestUtils qw( working_dir );

BEGIN {
    if ( DEBUG_ENABLED() ) {
        plan( tests => 7 );
    }
    else {
        plan( skip_all => 'DEBUG not enabled' );
    }
}

my $stderr_dumpfile = catfile( working_dir(), 'kino_garbage' );
unlink $stderr_dumpfile;
sysopen( STDERR, $stderr_dumpfile, O_CREAT | O_WRONLY | O_EXCL )
    or die "Failed to redirect STDERR";

DEBUG_PRINT("Roach Motel");
like( slurp_file($stderr_dumpfile), qr/Roach Motel/, "DEBUG_PRINT" );

ASSERT(1);
pass("ASSERT(true) didn't die");

SKIP: {
    skip( "Windows fork not supported by KS", 3 ) if $^O =~ /mswin/i;

    my $stderr_out = capture_debug( 'KinoSearch.xs', 'Borax' );
    like( $stderr_out, qr/Borax/, "DEBUG - file name" );

    $stderr_out
        = capture_debug( 'XS_KinoSearch__Util__Debug_DEBUG', "Strychnine" );
    like( $stderr_out, qr/Strychnine/, "DEBUG - function name" );

    $stderr_out = capture_debug( 'KinoS*', 'Raid' );
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
