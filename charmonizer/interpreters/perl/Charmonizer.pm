package Charmonizer;
use strict;
use warnings;

our $VERSION = 0.01;

use Carp;
use Config;
use File::Spec::Functions qw( rel2abs );

our %class_defaults = (
    modules        => [],
    out_path       => undef,
    compiler       => $Config{cc},
    charmonizer_fh => undef,
    clean_up       => 1,
);

sub new {
    my $class = shift;
    $class = ref($class) || $class;
    my $self = bless {%class_defaults}, $class;

    # verify params
    while (@_) {
        my ( $var, $val ) = ( shift, shift );
        confess "Unrecognized param: '$var'"
            unless exists $class_defaults{$var};
        $self->{$var} = $val;
    }
    confess "Missing required parameter 'out_path'"
        unless defined $self->{out_path};
    confess "No modules supplied" unless @{ $self->{modules} };

    # test for a working C compiler, without which all hope is lost
    open( my $charmonizer_fh, '>', '_charmonize.c' )
        or confess("Can't open '_charmonize.c': $!");
    print $charmonizer_fh "int main(){ return 0; }";
    close $charmonizer_fh or confess("Can't close '_charmonize.c': $!");
    system( $self->{compiler}, '-o', '_charmonize', '_charmonize.c' )
        and confess("Test compilation failed");

    # reopen _charmonize.c, clobbering previous test content
    open( $charmonizer_fh, '>', '_charmonize.c' )
        or confess("Can't open '_charmonize.c': $!");
    $self->{charmonizer_fh} = $charmonizer_fh;

    # parse and prepare all modules.
    my @modules;
    for my $module_path ( @{ $self->{modules} } ) {
        push @modules,
            Charmonizer::Module->new(
            charmonizer_fh => $charmonizer_fh,
            path           => $module_path,
            compiler       => $self->{compiler},
            );
    }
    $self->{modules} = \@modules;

    return $self;
}

sub run {
    my $self           = shift;
    my $charmonizer_fh = $self->{charmonizer_fh};

    # write _charmonize.c
    $_->run for @{ $self->{modules} };
    close $charmonizer_fh or confess("Can't close '_charmonize.c': $!");

    # compile _charmonize.c and capture its output
    system( $Config{cc}, '-o', '_charmonize', '_charmonize.c' )
        and confess("_charmonize.c failed to compile");
    my $command = rel2abs('_charmonize');
    open( my $out_fh, '>', $self->{out_path} )
        or confess("Can't open '$self->{out_path}': $!");
    print $out_fh `$command`;

    # clean up
    for ( '_charmonize.c', '_charmonize' ) {
        next unless -e $_;
        unlink $_ or confess("Couldn't unlink '$_': $!");
    }
}

package Charmonizer::Module;
use strict;
use warnings;

use Carp;
use File::Spec::Functions qw( rel2abs );

my $end_of_line   = qr/(?:\015\012|\012|\015|\z)/;
my $identifier    = qr/\b[a-zA-Z_][a-zA-Z0-9_]*\b/;
my $start_quote   = qr/CH_quote/;
my $end_quote     = qr/CH_end_quote/;
my $start_comment = qr#/\*#;
my $end_comment   = qr#\*/#;

sub new {
    my $unused = shift;
    my $self = bless {
        tree => [],      # more of a dowel, actually (no branches)
        code => undef,
        @_
        },
        __PACKAGE__;

    # if code wasn't supplied, get it from a file
    if ( !defined $self->{code} ) {
        open( my $module_fh, '<', $self->{path} )
            or confess("Can't open '$self->{path}': $!");
        $self->{code} = do { local $/; <$module_fh> };
        close $module_fh or confess("Can't close '$self->{path}': $!");
    }

    # parse now, rather than when run() is called.
    $self->_parse;

    return $self;
}

# Parse the source, checking for syntax errors.
sub _parse {
    my $self = shift;
    my $tree = $self->{tree};
    my $code = $self->_strip_comments( $self->{code} );
    my %inside;
    my $func_nick;
    my $args = {};
    my ( $param, $value ) = ( '', '' );
    my $line_number = 0;

    # consume copy of source line by line
LINE: while ( length $code and $code =~ s/(.*?$end_of_line)//s ) {
        my $line = $1;
        $line_number++;

        # append the line to a value if we're inside a quoted string.
        if ( $inside{quote} ) {
            if ( $line =~ s/^\s*CH_end_quote// ) {
                $inside{quote} = 0;
                $args->{$param} = $value;
                ( $param, $value ) = ( '', '' );
            }
            else {
                $value .= $line;
            }
            next LINE;
        }

        if ( $inside{function} ) {
            # if we've reached the end of a function, add a node to the tree
            if ( $line =~ s/^\s*CH_end_$func_nick// ) {
                push @$tree,
                    {
                    function => $inside{function},
                    args     => $args,
                    };
                $inside{function} = 0;
                undef $func_nick;
                $args = {};
            }
            # add a named parameter
            elsif ( $line =~ s/^\s*($identifier)\s+(.*?)\s*$// ) {
                ( $param, $value ) = ( $1, $2 );
                # check for start of quoted string
                if ( $value =~ s/^$start_quote// ) {
                    $inside{quote} = 1;
                    # check for quote close
                    if ( $value =~ s/$end_quote\s*$// ) {
                        $inside{quote} = 0;
                        $args->{$param} = $value;
                    }
                    else {
                        next LINE;
                    }
                }
                # no quoted string, so add the param
                else {
                    $args->{$param} = $value;
                }
            }
        }

        # start a function
        if ( $line =~ s/\s*(CH_(\w+))// ) {
            # don't allow nested functions
            if ( $inside{function} ) {
                confess(  "Parse error: already inside $inside{function} "
                        . "at line $line_number" );
            }
            $inside{function} = $1;
            $func_nick = $2;
        }

        # if there's anything other than whitespace left, it's a syntax error
        if ( $line =~ /\S/ ) {
            confess("Syntax error at $self->{path} line $line_number");
        }
    }

    # Require all modules to name their api version
    if ( $tree->[0]{function} ne "CH_api_version" ) {
        confess("$self->{path} doesn't begin with CH_api_version");
    }
}

# Remove comments from source, but preserve newlines (and thus line numbers).
sub _strip_comments {
    my ( $self, $code ) = @_;
    my $new_code    = '';
    my $line_number = 1;

    for ($code) {
        while ( length($_) ) {
            if (s/^(.*?)($start_quote|$start_comment)//s) {
                # append everything up to the quote/comment
                my ( $preceding_code, $delimiter ) = ( $1, $2 );
                $new_code .= $preceding_code;
                $line_number += $self->_count_newlines($preceding_code);

                if ( $delimiter eq 'CH_quote' ) {
                    # it's a quote, so include the quote and the delimiters
                    $new_code .= $delimiter;
                    s/^(.*?$end_quote)//s
                        or confess(
                              "Unterminated quote starting at $self->{path} "
                            . "line $line_number" );
                    $new_code .= $1;
                }
                else {
                    # it's a comment, so strip it
                    s/^(.*?$end_comment)//s
                        or confess(
                        "Unterminated comment starting at $self->{path} "
                            . "line $line_number" );
                    $line_number += $self->_count_newlines($1);
                }
            }
            # once we've passed the last comment/quote, concat the rest
            else {
                $new_code .= $_;
                $_ = '';
            }
        }
    }

    return $new_code;
}

sub run {
    my $self = shift;

    # process nodes linearly
    for my $node ( @{ $self->{tree} } ) {
        my $func = $node->{function};
        $self->$func( $node->{args} );
    }
}

# Helper function for determining line number.
sub _count_newlines {
    my $code         = shift;
    my $num_newlines = 0;
    while ( length $code and $code =~ s/.*?$end_of_line//g ) {
        $num_newlines++;
    }
    return $num_newlines;
}

sub CH_api_version {
    my ( $self, $args ) = @_;
    if ( int( $args->{version} ) > int($Charmonizer::VERSION) ) {
        confess(  "File '$self->{path}' version $args->{api_version} "
                . "incompatible with Charmonizer version $Charmonizer::VERSION"
        );
    }
}

sub CH_append_raw {
    my ( $self, $args ) = @_;
    my $charmonizer_fh = $self->{charmonizer_fh};
    confess(  "Missing required argument 'string' at $self->{path} "
            . "line $args->{line_number}" )
        unless defined $args->{string};
    print $charmonizer_fh $args->{string};
}

sub CH_append_output {
    my ( $self, $args ) = @_;
    my $charmonizer_fh = $self->{charmonizer_fh};

    # print the test code to a probe C file
    open( my $probe_fh, '>', '_ch_probe.c' )
        or confess("Can't open '_ch_probe.c': $!");
    print $probe_fh $args->{source};
    close $probe_fh or confess("Can't close '_ch_probe.c': $!");

    # suppress error messages, since we know some of these will fail.
    open( ERRCOPY, ">&STDERR" ) or confess $!;
    close STDERR or confess $!;
    my $succeeded
        = system( $self->{compiler}, '-o', '_ch_probe', '_ch_probe.c' ) == 0;
    open( STDERR, ">&ERRCOPY" ) or confess $!;
    close ERRCOPY or confess $!;

    # append the output of the compiled program to the
    if ($succeeded) {
        my $command = rel2abs('_ch_probe');
        print $charmonizer_fh `$command`;
    }

    # clean up probe files
    for ( '_ch_probe', '_ch_probe.c' ) {
        next unless -e $_;
        unlink $_ or confess "Can't unlink file '$_': $!";
    }
}

1;

__END__

=head1 NAME

Charmonizer - Write portable C header files.

=head1 SYNOPSIS

    my $charmonizer = Charmonizer->new(
        modules => [
            'modules/start.charm',
            'modules/integers.charm',
            'modules/end.charm',
        ],
        out_path => 'charmonized.h',
    );
    $charmonizer->run;

=head1 DESCRIPTION

The Charmonizer class supplies an interpreter for the Charmonizer language,
which is used to write portable C header files.  There are only two methods,
because all the functionality is in the Charmonizer language itself.

=head1 METHODS

=head2 new

Constructor.  Takes hash-style labeled parameters.

=over

=item *

B<modules> -- an arrayref, where each element in the array is a filepath
pointing to a Charmonizer module file (typically ending in ".charm").

=item *

B<out_path> -- indicates where the final output from the Charmonizer program
should end up.

=item *

B<compiler> -- The command used to invoke the C compiler on your system.  The
default is C<$Config{cc}> -- see the docs for L<Config>.

=back

=head2 run

Run the Charmonizer program, generating the header file.

=head1 AUTHOR

Marvin Humphrey, E<lt>marvin at rectangular dot comE<gt>

=head1 COPYRIGHT AND LICENSE

    /**
     * Copyright 2006 The Apache Software Foundation
     *
     * Licensed under the Apache License, Version 2.0 (the "License");
     * you may not use this file except in compliance with the License.
     * You may obtain a copy of the License at
     *
     *     http://www.apache.org/licenses/LICENSE-2.0
     *
     * Unless required by applicable law or agreed to in writing, software
     * distributed under the License is distributed on an "AS IS" BASIS,
     * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
     * implied.  See the License for the specific language governing
     * permissions and limitations under the License.
     */

=cut

