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

package LucyX::Index::LongFieldSim;
use base qw( Lucy::Index::Similarity );
our $VERSION = '0.006002';
$VERSION = eval $VERSION;

sub length_norm {
    my ( $self, $num_tokens ) = @_;
    $num_tokens = $num_tokens < 100 ? 100 : $num_tokens;
    return 1 / sqrt($num_tokens);
}

1;

__END__

__POD__

=head1 NAME

LucyX::Index::LongFieldSim - Similarity optimized for long fields.

=head1 SYNOPSIS

    package MySchema::body;
    use base qw( Lucy::Plan::FullTextType );
    use LucyX::Index::LongFieldSim;
    sub make_similarity { LucyX::Index::LongFieldSim->new }

=head1 DESCRIPTION

Apache Lucy's default L<Similarity|Lucy::Index::Similarity>
implmentation produces a bias towards extremely short fields.

    Lucy::Index::Similarity
    
    | more weight
    | *
    |  **  
    |    ***
    |       **********
    |                 ********************
    |                                     *******************************
    | less weight                                                        ****
    |------------------------------------------------------------------------
      fewer tokens                                              more tokens

LongFieldSim eliminates this bias.

    LucyX::Index::LongFieldSim
    
    | more weight
    | 
    |    
    |    
    |*****************
    |                 ********************
    |                                     *******************************
    | less weight                                                        ****
    |------------------------------------------------------------------------
      fewer tokens                                              more tokens

In most cases, the default bias towards short fields is desirable.  For
instance, say you have two documents:

=over

=item *

"George Washington"

=item *

"George Washington Carver"

=back

If a user searches for "george washington", we want the exact title match to
appear first.  Under the default Similarity implementation it will, because
the "Carver" in "George Washington Carver" dilutes the impact of the other two
tokens.  

However, under LongFieldSim, the two titles will yield equal scores.  That
would be bad in this particular case, but it could be good in another.  

     "George Washington Carver is cool."

     "George Washington Carver was born on the eve of the US Civil War, in
     1864.  His exact date of birth is unknown... Carver's research in crop
     rotation revolutionized agriculture..."

The first document is succinct, but useless.  Unfortunately, the default
similarity will assess it as extremely relevant to a query of "george
washington carver".  However, under LongFieldSim, the short-field bias is
eliminated, and the addition of other mentions of Carver's name in the second
document yield a higher score and a higher rank.

=cut


