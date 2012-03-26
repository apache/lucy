#!/usr/bin/perl

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
use File::Spec::Functions qw( catfile rel2abs tmpdir );
use File::Find qw( find );
use File::Temp qw( mktemp );
use Getopt::Long;
use Fcntl;

my $ignore = qr/(
      \.svn
    | \.git
    | modules.analysis.snowstem.source
    | perl.sample
    )/x;
my $scratch_template = catfile( tmpdir(), 'lucytidy_scratch_XXXXXX' );
my $scratch = mktemp($scratch_template);
END { unlink $scratch }

# Parse command-line options.
my $astyle   = 'astyle';
my $perltidy = 'perltidy';
my $suffix   = 'tdy';
GetOptions(
    'perltidy=s' => \$perltidy,
    'astyle=s'   => \$astyle,
    'suffix:s'   => \$suffix
);
my $start = $ARGV[0];
die "Usage: run_astyle.pl [options] PATH" unless $start;

# Find astylerc and perltidyrc files.
my $top_dir = rel2abs(__FILE__);
$top_dir =~ s/\Wdevel.*//;
my $astylerc = catfile( $top_dir, qw( devel conf astylerc ) );
die "Can't find astylerc file" unless -f $astylerc;
my $perltidyrc = catfile( $top_dir, qw( devel conf perltidyrc ) );
die "Can't find perltidyrc file" unless -f $perltidyrc;

# Find/verify astyle and perltidy executables.
my $astyle_version_output = `$astyle --version 2>&1`;
my $min_astyle            = 2.01;
my ($astyle_version)
    = defined $astyle_version_output
    ? $astyle_version_output =~ /version\s+([\d.]+)/i
    : (undef);
if ( !defined $astyle_version || $astyle_version < $min_astyle ) {
    print "No astyle version >= $min_astyle -- "
        . "C source files will be skipped...\n";
    undef $astyle;
}
my $perltidy_version_output = `$perltidy --version 2>&1`;
my $min_perltidy            = "20090616";
my ($perltidy_version)
    = defined $perltidy_version_output
    ? $perltidy_version_output =~ /\sv(\d+)\s/i
    : (undef);
if ( !defined $perltidy_version || $perltidy_version < $min_perltidy ) {
    print "No perltidy version >= $min_perltidy -- "
        . "Perl source files will be skipped...\n";
    undef $perltidy;
}

# Process files.
find( { no_chdir => 1, wanted => \&process_file, }, $start );
exit;

sub process_file {
    my $path = $File::Find::name;
    if ( !-f $path ) {
        return;
    }
    elsif ( $path =~ $ignore ) {
        return;
    }
    elsif ( $path =~ /\.(c|h)$/ ) {
        process_c($path);
    }
    elsif ( $path =~ /\.(pl|plx|PL|pm|t)$/i ) {
        process_perl($path);
    }
}

sub process_c {
    my $path = shift;
    if ( !defined $astyle ) {
        print "No astyle version >= $min_astyle, skipping $path\n";
        return;
    }
    my $content      = read_file($path);
    my $orig_content = $content;

    # Our __cplusplus header wrappers cause AStyle to indent everything
    # between them.  Defeat this behavior by installing placeholders that
    # don't contain brackets.
    $content =~ s/(__cplusplus\s+)extern\s*"C"\s*\{/$1EXTERN_C_OPEN/g;
    $content =~ s/(__cplusplus\s+)\}/$1EXTERN_C_CLOSE/g;

    # Prevent AStyle from forcing all preprocessor directives into column 1.
    # This hack works by filling leading spaces before preprocessor directives
    # with '#'.  When AStyle sees multiple '#' symbols beginning a line, it
    # gives up and leaves the line untouched.
    $content =~ s/^([ ]+)#/'#' x (length($1) + 1)/mge;

    # AStyle has a bug regarding statement continuation lines which break
    # around the assignment operator.  We can fool AStyle into doing the right
    # thing by tacking on an extra '=' to the line before (a C syntax error).
    $content =~ s!(\n\s+= )! = // HAKK$1!g;

    # The same AStyle directive which allows for uncuddled elses also has the
    # undesirable effect of detaching the "while" in a do-while loop from its
    # closing bracket.  We can thwart this behavior by munging the 'while'
    # symbol.
    $content =~ s!\}[ ]+while!} wHiLe!g;

    # AStyle sometimes confuses dereference operators for multiplication
    # operators and pads them.  This hack prevents it from turning
    # '(Foo*volatile*)' into '(Foo * volatile*)'.
    $content =~ s!\*volatile\*!STARVOLAT*!g;

    # AStyle has a bad interaction with Charmonizer's QUOTE macro, because it
    # can't handle e.g. implicitly stringified brackets:
    #
    #   QUOTE(    }                                               )
    #
    # We solve this problem by swapping in innocuous placeholders for the
    # contents of all QUOTE macros.
    my %quote_placeholders;
    my $counter = 0;
    while ( $content =~ /\bQUOTE\(/ ) {
        $counter++;
        my $placeholder = "KWOTE$counter()";
        die "Already found placeholder '$placeholder'"
            if $content =~ /$placeholder/;
        $content =~ s/(QUOTE\(.*\))(.*?)$/$placeholder$2/m
            or die "no match ('$path')";
        $quote_placeholders{$placeholder} = $1;
    }

    # Write out the prepped file and run AStyle on it.
    write_file( $scratch, $content );
    system(
        "$astyle --options=$astylerc --suffix=none --mode=c --quiet $scratch")
        and die "astyle failed";

    # Undo all of the transforms.
    $content = read_file($scratch);
    $content =~ s/EXTERN_C_OPEN/extern "C" {/g;
    $content =~ s/EXTERN_C_CLOSE/}/g;
    $content =~ s/^(#+)#/" " x length($1) . '#'/mge;
    $content =~ s! = // HAKK!!g;
    $content =~ s!wHiLe[ ]*\(!while (!g;
    $content =~ s!STARVOLAT!*volatile!g;
    while ( my ( $placeholder, $orig ) = each %quote_placeholders ) {
        $content =~ s/\Q$placeholder/$orig/
            or die "Can't match placeholder '$placeholder'";
    }

    # Write out the final file if it's been changed.
    if ( $content eq $orig_content ) {
        print "Unchanged: $path\n";
    }
    else {
        print "Tidied:    $path\n";
        my $out = $suffix ? "$path.$suffix" : $path;
        write_file( $out, $content );
    }
}

sub process_perl {
    my $path = shift;
    if ( !defined $perltidy ) {
        print "Skipped:   $path\n";
        return;
    }
    unlink $scratch;
    system("$perltidy -pro=$perltidyrc -o=$scratch $path")
        and die "perltidy failed";

    # Write out the final file if it's been changed.
    my $orig_content = read_file($path);
    my $tidied       = read_file($scratch);
    if ( $tidied eq $orig_content ) {
        print "Unchanged: $path\n";
    }
    else {
        my $out = $suffix ? "$path.$suffix" : $path;
        print "Tidied:    $out\n";
        write_file( $out, $tidied );
    }
}

sub read_file {
    my $path = shift;
    open( my $fh, '<', $path ) or confess("Can't open '$path': $!");
    local $/;
    return <$fh>;
}

sub write_file {
    my ( $path, $content ) = @_;
    unlink $path;
    sysopen( my $fh, $path, O_CREAT | O_EXCL | O_WRONLY )
        or confess("Can't open '$path': $!");
    print $fh $content;
    close $fh or confess("Close failed for '$path': $!");
}

__END__

=head1 NAME

lucytidy.pl - Auto-format Lucy code.

=head1 SYNOPSIS

    lucytidy.pl [options] PATH

=head1 DESCRIPTION

Lucy uses various automatic code formatters for the different languages it
supports: AStyle, a.k.a. Artistic Style, for C code, Perltidy for Perl, etc.
This wrapper script walks the directory structure pointed to by the supplied
C<PATH> and decides based on file type which formatter to run.  It also works
around some formatter quirks and bugs so that we don't have to live with
compromises in our actual code.

=head1 OPTIONS

=head2 --suffix

     lucytidy.pl --suffix="" PATH

The suffix for the tidied file.  The default is "tdy".  Supplying an empty
string causes the original file to be overwritten.


=head2 --astyle

    lucytidy.pl --astyle=/path/to/alternative/astyle PATH

Specify the location of the astyle executable.

=head2 --perltidy

    lucytidy.pl --perltidy=/path/to/alternative/perltidy PATH

Specify the location of the perltidy executable.

=cut

