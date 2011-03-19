#!/usr/bin/env perl

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
use Carp;
use FindBin;
use Sys::Hostname;
use File::Spec::Functions qw( catdir canonpath );
use Net::SMTP;

my $config = {
    src           => canonpath( catdir( $FindBin::Bin, '../../' ) ),
    verbose       => 0,
    email_to      => undef,
    email_from    => getpwuid($<) . '@' . hostname(),
    email_subject => 'Lucy Smoke Test Report ' . localtime(),
    mailhost      => 'localhost',
    test_target => 'test',    # could also be 'test_valgrind' if on Linux
};

if (@ARGV) {
    my $config_file = shift @ARGV;
    open( my $fh, '<', $config_file ) or die "Can't open '$config_file': $!";
    while ( defined( my $line = <$fh> ) ) {
        $line =~ s/#.*//;
        next unless $line =~ /^(.*?)=(.*)$/;
        my ( $key, $value ) = ( $1, $2 );
        for ( $key, $value ) {
            s/^\s*//;
            s/\s*$//;
        }
        $config->{$key} = $value;
    }
    close $fh or die "Can't close '$config_file': $!";
}

if ( !$config->{src} ) {
    croak "no 'src' in config -- check your syntax";
}
if ( !-d $config->{src} ) {
    croak "no such dir: $config->{src}";
}

my $test_target = $config->{test_target};
my $dir         = $config->{src};
my $perl_info   = get_out("$^X -V");
my $sys_info    = get_out('uname -a');
system( 'svn', 'up', $dir ) and croak "can't svn update $dir:\n";
chdir "$dir" or croak "can't chdir to $dir: $!";
chdir 'perl' or croak "can't chdir to perl: $!";
run_quiet("./Build clean") if -f 'Build';
run_quiet("$^X Build.PL");
run_quiet("$^X Build");
my $test_info = get_out("./Build $test_target");

if ( should_send_smoke_signal($test_info) ) {

    my $msg = <<EOF;
Looks like one or more tests failed:
$test_info
$sys_info
$perl_info
EOF
    $msg .= `svn info $dir`;

    if ( $ENV{SMOKE_TEST} ) {
        print $msg . "\n";
    }
    elsif ( $config->{email_to} ) {
        my $smtp = Net::SMTP->new( $config->{mailhost} );
        $smtp->mail( $config->{email_from} );
        $smtp->to( $config->{email_to} );
        $smtp->data();
        $smtp->datasend("$msg\n");
        $smtp->dataend();
        $smtp->quit;
    }
}
elsif ( $config->{verbose} ) {
    print "All tests pass.\n";
    print `svn info $dir`;
}

exit;

sub should_send_smoke_signal {
    return 1 if $_[0] =~ m/fail/i;
    return 1 if $? != 0;
}

sub run_quiet {
    my $cmd = shift;
    system("$cmd 2>/dev/null 1>/dev/null") and croak "$cmd failed: $!";
}

sub get_out {
    my $cmd = shift;
    return join( '', `$cmd 2>&1` );
}

__END__

=head1 NAME

smoke.pl - Lucy smoke test script

=head1 SYNOPSIS

 perl devel/bin/smoke.pl [path/to/config_file]

=head1 DESCRIPTION

By default, smoke.pl updates to the latest SVN version of the branch within which it resides
and runs a clean build and test suite. If there are any test failures, a full
system and test summary is printed to stdout.

You may specify an alternate path to test in an ini-formatted config file. 
Use the 'src' config option to specify a path. Example:

 src = /path/to/checked/out/lucy/branch
 email_to = me@example.com
 email_from = me@example.com

By default, smoke.pl will only print output if there are errors. To see output
if all tests pass, specify a true 'verbose' flag in your config file.

 verbose = 1

=cut

