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
use SVN::Class;
use FindBin;
use JSON::XS;
use Email::Stuff;
use Sys::Hostname;

my $config = {
    src => Path::Class::Dir->new( $FindBin::Bin, '../../' )->absolute->stringify,
    verbose       => 0,
    email_to      => undef,
    email_from    => getpwuid($<) . '@' . hostname(),
    email_subject => 'Lucy Smoke Test Report ' . localtime(),
    test_target => 'test',    # could also be 'test_valgrind' if on Linux
};

if (@ARGV) {
    my $config_file   = Path::Class::File->new( shift @ARGV );
    my $supplied_conf = decode_json( $config_file->slurp );
    $config = { %$config, %$supplied_conf };
}

if (!$config->{src}) {
    croak "no 'src' in config -- check your syntax";
}
if (! -d $config->{src}) {
    croak "no such dir: $config->{src}";
}

my $test_target = $config->{test_target};
my $dir         = svn_dir($config->{src});
my $perl_info   = get_out("$^X -V");
my $sys_info    = get_out('uname -a');
$dir->update or croak "can't svn update $dir:\n" . $dir->errstr;
chdir "$dir" or croak "can't chdir to $dir: $!";
chdir 'perl' or croak "can't chdir to perl: $!";
run_quiet("./Build clean") if -f 'Build';
run_quiet("$^X Build.PL");
run_quiet("$^X Build");
my $test_info = get_out("./Build $test_target");

if (should_send_smoke_signal($test_info)) {

    my $msg =<<EOF;
Looks like one or more tests failed:
$test_info
$sys_info
$perl_info
EOF
    $msg .= $dir->info->dump;

    if ($ENV{SMOKE_TEST}) {
        print $msg . "\n";
    }
    elsif ( $config->{email_to} ) {
        Email::Stuff->from     ( $config->{email_from}    )
                    ->to       ( $config->{email_to}      )
                    ->subject  ( $config->{email_subject} )
                    ->text_body( $msg                     )
                    ->send;             
    }
}
elsif ($config->{verbose}) {
    print "All tests pass.\n";
    print $dir->info->dump;
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
    return join('', `$cmd 2>&1`);
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

You may specify an alternate path to test in a JSON-formatted config file. 
Use the 'src' config option to specify a path. Example:

 { 'src' : '/path/to/checked/out/lucy/branch' }

By default, smoke.pl will only print output if there are errors. To see output
if all tests pass, specify a true 'verbose' flag in your config file.

 { 'verbose' : 1 }


=head1 REQUIREMENTS

SVN::Class, JSON::XS, FindBin, Carp

=cut

