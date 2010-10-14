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

sub _new {
    my $either = shift;
    verify_args( \%new_PARAMS, @_ );
    return bless { \%new_PARAMS, @_ };
}

sub parse {
    my ( $either, $text ) = @_;

    my ( @param_names, @param_docs );
    my $self = bless {
        brief       => undef,
        long        => undef,
        param_names => \@param_names,
        param_docs  => \@param_docs,
        retval      => undef,
        },
        ref($either) || $either;

    # Strip comment open, close, and left border.
    $text =~ s/\A\s*\/\*\*\s+//;
    $text =~ s/\s+\*\/\s*\Z//;
    $text =~ s/^\s*\* ?//gm;

    # Extract the brief description.
    $text =~ /^(.+?\.)(\s+|\Z)/s
        or confess("Can't find at least one descriptive sentence in '$text'");
    $self->{brief} = $1;

    # terminated by @, empty line, or string end.
    my $terminator = qr/((?=\@)|\n\s*\n|\Z)/;

    # Extract @param, @return directives.
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
    if ( $text =~ s/^\s*\@return\s+(.*?)$terminator//sm ) {
        $self->{retval} = $1;
    }

    $text =~ s/^\s*//;
    $text =~ s/\s*$//;
    $self->{description} = $text;

    $text =~ s/^(.+?\.)(\s+|\Z)//s;    # zap brief
    $text =~ s/^\s*//;
    $self->{long} = $text;

    return $self;
}

sub get_param_names { shift->{param_names} }
sub get_param_docs  { shift->{param_docs} }
sub get_retval      { shift->{retval} }
sub get_brief       { shift->{brief} }
sub get_long        { shift->{long} }
sub get_description { shift->{description} }

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

=head1 COPYRIGHT AND LICENSE

Copyright 2008-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify it under
the same terms as Perl itself.

=cut

