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

package LucyX::Search::MockMatcher;
use Lucy;
our $VERSION = '0.006002';
$VERSION = eval $VERSION;

sub new {
    my ( $either, %args ) = @_;
    confess("Missing doc_ids") unless ref( $args{doc_ids} ) eq 'ARRAY';
    my $doc_ids = Lucy::Object::I32Array->new( ints => $args{doc_ids} );
    my $size = $doc_ids->get_size;
    my $scores;
    if ( ref( $args{scores} ) eq 'ARRAY' ) {
        confess("Mismatch between scores and doc_ids array sizes")
            unless scalar @{ $args{scores} } == $size;
        $scores = Clownfish::Blob->new(
            pack( "f$size", @{ $args{scores} } ) );
    }

    return $either->_new(
        doc_ids => $doc_ids,
        scores  => $scores,
    );
}

1;

__END__



=head1 NAME

LucyX::Search::MockMatcher - Matcher with arbitrary docs and scores.

=head1 DESCRIPTION 

Used for testing combining L<Matchers|Lucy::Search::Matcher> such as
ANDMatcher, MockMatcher allows arbitrary match criteria to be supplied,
obviating the need for clever index construction to cover corner cases.

MockMatcher is a testing and demonstration class; it is unsupported.

=head1 CONSTRUCTORS

=head2 new( [I<labeled params>] )

=over

=item *

B<doc_ids> - A sorted array of L<doc_ids|Lucy::Docs::DocIDs>.

=item *

B<scores> - An array of scores, one for each doc_id.

=back

=cut
