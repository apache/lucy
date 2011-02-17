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

package Clownfish::DocuComment;
use Carp;

our %new_PARAMS = (
    description => undef,
    brief       => undef,
    long        => undef,
    param_names => undef,
    param_docs  => undef,
    retval      => undef,
);

sub parse {
    my ( $either, $text ) = @_;
    my $class_name = ref($either) || $either;

    # Strip comment open, close, and left border.
    $text =~ s/^\s*//;
    $text =~ s/\s*$//;
    $text = strip($text);

    # Extract the brief description.
    $text =~ /^(.+?\.)(\s+|\Z)/s
        or confess("Can't find at least one descriptive sentence in '$text'");
    my $brief = $1;

    # terminated by @, empty line, or string end.
    my $terminator = qr/((?=\@)|\n\s*\n|\Z)/;

    # Extract @param, @return directives.
    my ( @param_names, @param_docs );
    while (
        $text =~ s/^\s*
        \@param\s+
        (\w+)   # param name
        \s+
        (.*?)   # param description
        \s*
        $terminator
      //xsm
        )
    {
        push @param_names, $1;
        push @param_docs,  $2;
    }
    my $retval;
    if ( $text =~ s/^\s*\@return\s+(.*?)$terminator//sm ) {
        $retval = $1;
    }

    $text =~ s/^\s*//;
    $text =~ s/\s*$//;
    my $description = $text;

    $text =~ s/^(.+?\.)(\s+|\Z)//s;    # zap brief
    $text =~ s/^\s*//;
    my $long = $text;

    return $class_name->_new( $description, $brief, $long, \@param_names,
        \@param_docs, $retval );
}

1;

__END__

__POD__

=head1 NAME

Clownfish::DocuComment - Formatted comment a la Doxygen.

=head1 SYNOPSIS

    my $text = <<'END_COMMENT';
    /** Brief description.
     *
     * Start the long description.  More long description.
     * 
     * @param foo A Foo.
     * @param bar A Bar.
     * @return a return value.
     */
    END_COMMENT
    my $docucomment = Clownfish::DocuComment->parse($text);

=head1 CONSTRUCTORS 

=head2 parse 

    my $self = Clownfish::DocuComment->parse($text);

Parse comment text.

=head2 new

    my $self = Clownfish::DocuComment->new(
        description => "Brief.  Start long.  More long.",
        brief       => "Brief.",
        long        => "Long start. More long.",
        param_names => \@param_names,
        param_docs  => \@param_docs,
        retval      => "a return value."
    );

=over

=item * B<description> - The complete description. 

=item * B<brief> - The first sentence of the description (a "brief"
description).

=item * B<long> - The description minus the first sentence.

=item * B<param_names> - An array of param names.

=item * B<param_docs> - An array containing a blurb for each param name.

=item * B<retval> - Return value.

=back

=head1 METHODS

=head2 get_description get_brief get_long get_param_names get_param_docs get_retval

Accessors.

=cut

