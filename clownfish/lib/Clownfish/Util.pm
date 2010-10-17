use strict;
use warnings;

package Clownfish::Util;
use base qw( Exporter );
use Scalar::Util qw( blessed );
use Carp;
use Fcntl;

our @EXPORT_OK = qw(
    slurp_file
    current
    strip_c_comments
    verify_args
    a_isa_b
    write_if_changed
);

sub slurp_file {
    my $path = shift;
    open( my $fh, '<', $path ) or confess("Can't open '$path': $!");
    local $/;
    return <$fh>;
}

sub current {
    my ( $orig, $dest ) = @_;
    my $bubble_time = time;
    $orig = [$orig] unless ref($orig) eq 'ARRAY';
    $dest = [$dest] unless ref($dest) eq 'ARRAY';

    # If a destination file doesn't exist, we're not current.
    for (@$dest) {
        return 0 unless -e $_;
    }

    # Find the oldest file from the destination group.
    for (@$dest) {
        my $candidate = ( stat($_) )[9];
        $bubble_time = $candidate if $candidate < $bubble_time;
    }

    # If any source file is newer than the oldest dest, we're not current.
    for (@$orig) {
        confess "Missing source file '$_'" unless -e $_;
        my $candidate = ( stat($_) )[9];
        return 0 if $candidate > $bubble_time;
    }

    # Current!
    return 1;
}

sub strip_c_comments {
    my $c_code = shift;
    $c_code =~ s#/\*.*?\*/##gsm;
    return $c_code;
}

sub verify_args {
    my $defaults = shift;    # leave the rest of @_ intact

    # Verify that args came in pairs.
    if ( @_ % 2 ) {
        my ( $package, $filename, $line ) = caller(1);
        $@ = "Parameter error: odd number of args at $filename line $line\n";
        return 0;
    }

    # Verify keys, ignore values.
    while (@_) {
        my ( $var, undef ) = ( shift, shift );
        next if exists $defaults->{$var};
        my ( $package, $filename, $line ) = caller(1);
        $@ = "Invalid parameter: '$var' at $filename line $line\n";
        return 0;
    }

    return 1;
}

sub a_isa_b {
    my ( $thing, $class ) = @_;
    return 0 unless blessed($thing);
    return $thing->isa($class);
}

sub write_if_changed {
    my ( $path, $content ) = @_;
    my $write = 1;
    if ( -e $path ) {
        open( my $fh, '<', $path ) or confess "Can't open '$path': $!";
        my $orig = do { local $/; <$fh> };
        return if $orig eq $content;
    }
    unlink $path;
    sysopen( my $fh, $path, O_CREAT | O_EXCL | O_WRONLY )
        or confess "Can't open '$path': $!";
    print $fh $content;
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Util - Miscellaneous helper functions.

=head1 DESCRIPTION

Clownfish::Util provides a few convenience functions used internally by
other Clownfish modules.

=head1 FUNCTIONS

=head2 slurp_file

    my $foo_contents = slurp_file('foo.txt');

Open a file, read it in, return its contents.  Assumes either binary data or
text with an encoding of Latin-1.

=head2 current

    compile('foo.c') unless current( 'foo.c', 'foo.o' );

Given two elements, which may be either scalars or arrays, verify that
everything in the second group exists and was created later than anything in
the first group.

=head2 verify_args

    verify_args( \%defaults, @_ ) or confess $@;

Verify that named parameters exist in a defaults hash.  Returns false and sets
$@ if a problem is detected.

=head2 strip_c_comments

    my $c_minus_comments = strip_c_comments($c_source_code);

Quick 'n' dirty stripping of C comments.  Will massacre stuff like comments
embedded in string literals, so watch out.

=head2 write_if_changed

    write_if_changed( $path, $content );

Test whether there's a file at C<$path> which already matches C<$content>
exactly.  If something has changed, write the file.  Otherwise do nothing (and
avoid bumping the file's modification time).

=cut

