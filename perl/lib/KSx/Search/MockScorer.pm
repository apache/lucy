use strict;
use warnings;
use KinoSearch;

package KSx::Search::MockScorer;
BEGIN { our @ISA = qw( KSx::Search::MockMatcher ) }

sub new {
    my ( $either, %args ) = @_;
    confess("Missing doc_ids") unless ref( $args{doc_ids} ) eq 'ARRAY';
    my $doc_ids = KinoSearch::Object::I32Array->new( ints => $args{doc_ids} );
    my $size = $doc_ids->get_size;
    my $scores;
    if ( ref( $args{scores} ) eq 'ARRAY' ) {
        confess("Mismatch between scores and doc_ids array sizes")
            unless scalar @{ $args{scores} } == $size;
        $scores = KinoSearch::Object::ByteBuf->new(
            pack( "f$size", @{ $args{scores} } ) );
    }

    return $either->_new(
        doc_ids => $doc_ids,
        scores  => $scores,
    );
}

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KSx::Search::MockMatcher",
    bind_constructors => ["_new|init"],
);

__POD__

=head1 NAME

KSx::Search::MockScorer - Matcher with arbitrary docs and scores.

=head1 DESCRIPTION 

Used for testing combining scorers such as ANDScorer, MockScorer allows
arbitrary match criteria to be supplied, obviating the need for clever index
construction to cover corner cases.

MockScorer is a testing and demonstration class; it is unsupported.

=head1 CONSTRUCTORS

=head2 new( [I<labeled params>] )

=over

=item *

B<doc_ids> - A sorted array of L<doc_ids|KinoSearch::Docs::DocIDs>.

=item *

B<scores> - An array of scores, one for each doc_id.

=back

=head1 COPYRIGHT

Copyright 2007-2010 Marvin Humphrey

=head1 LICENSE, DISCLAIMER, BUGS, etc.

See L<KinoSearch> version 0.30.

=cut
