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

package Lucy::Simple;
use Lucy;
our $VERSION = '0.004000';
$VERSION = eval $VERSION;
use Carp;
use Scalar::Util qw( weaken reftype refaddr );

use Lucy::Plan::Schema;
use Lucy::Analysis::EasyAnalyzer;
use Lucy::Index::Indexer;
use Lucy::Search::IndexSearcher;

my %obj_cache;

sub new {
    my ( $either, %args ) = @_;
    my $path     = delete $args{path};
    my $language = lc( delete $args{language} );
    confess("Missing required parameter 'path'") unless defined $path;
    confess("Invalid language: '$language'")
        unless $language =~ /^(?:da|de|en|es|fi|fr|it|nl|no|pt|ru|sv)$/;
    my @remaining = keys %args;
    confess("Invalid params: @remaining") if @remaining;
    my $self = bless {
        type     => undef,
        schema   => undef,
        indexer  => undef,
        searcher => undef,
        hits     => undef,
        language => $language,
        path     => $path,
        },
        ref($either) || $either;

    # Cache the object for later clean-up.
    weaken( $obj_cache{ refaddr $self } = $self );

    return $self;
}

sub _lazily_create_indexer {
    my $self = shift;
    if ( !defined $self->{indexer} ) {
        # Get type and schema
        my $schema;
        my $reader = Lucy::Index::PolyReader->open( index => $self->{path} );
        if ( !@{ $reader->seg_readers } ) {
            # index is empty, create new schema and type
            $schema = Lucy::Plan::Schema->new;
            my $analyzer = Lucy::Analysis::EasyAnalyzer->new(
                language => $self->{language}, );
            $self->{type}
                = Lucy::Plan::FullTextType->new( analyzer => $analyzer, );
        }
        else {
            # get schema from reader
            $schema = $reader->get_schema;
            my $field = $schema->all_fields->[0];
            $self->{type} = $schema->fetch_type($field);
        }
        $self->{schema}  = $schema;
        $self->{indexer} = Lucy::Index::Indexer->new(
            schema => $schema,
            index  => $self->{path},
        );
    }
}

sub add_doc {
    my ( $self, $hashref ) = @_;
    croak("add_doc requires exactly one argument: a hashref")
        unless ( @_ == 2 and reftype($hashref) eq 'HASH' );
    $self->_lazily_create_indexer;
    my $schema = $self->{schema};
    my $type   = $self->{type};
    $schema->spec_field( name => $_, type => $type ) for keys %$hashref;
    $self->{indexer}->add_doc($hashref);
}

sub _finish_indexing {
    my $self = shift;

    # Don't bother to throw an error if index not modified.
    if ( defined $self->{indexer} ) {
        $self->{indexer}->commit;

        # Trigger searcher and indexer refresh.
        undef $self->{indexer};
        undef $self->{searcher};
    }
}

sub search {
    my ( $self, %args ) = @_;

    # Flush recent adds; lazily create searcher.
    $self->_finish_indexing;
    if ( !defined $self->{searcher} ) {
        $self->{searcher}
            = Lucy::Search::IndexSearcher->new( index => $self->{path} );
    }

    $self->{hits} = $self->{searcher}->hits(%args);

    return $self->{hits}->total_hits;
}

sub next {
    my $self = shift;
    return unless defined $self->{hits};

    # Get the hit, bail if hits are exhausted.
    my $hit = $self->{hits}->next;
    if ( !defined $hit ) {
        undef $self->{hits};
        return;
    }

    return $hit;
}

sub DESTROY {
    for (shift) {
        $_->_finish_indexing;
        delete $obj_cache{ refaddr $_ };
    }
}

END {
    # Finish indexing for any objects that still exist, since, if we wait
    # until global destruction, our Indexer might no longer exist,
    # (see bug #32689)
    $_->_finish_indexing for values %obj_cache;
}

1;

__END__

__POD__

=head1 NAME

Lucy::Simple - Basic search engine.

=head1 SYNOPSIS

First, build an index of your documents.

    my $index = Lucy::Simple->new(
        path     => '/path/to/index/'
        language => 'en',
    );

    while ( my ( $title, $content ) = each %source_docs ) {
        $index->add_doc({
            title    => $title,
            content  => $content,
        });
    }

Later, search the index.

    my $total_hits = $index->search( 
        query      => $query_string,
        offset     => 0,
        num_wanted => 10,
    );

    print "Total hits: $total_hits\n";
    while ( my $hit = $index->next ) {
        print "$hit->{title}\n",
    }

=head1 DESCRIPTION

Lucy::Simple is a stripped-down interface for the L<Apache Lucy|Lucy> search
engine library.  

=head1 METHODS 

=head2 new

    my $lucy = Lucy::Simple->new(
        path     => '/path/to/index/',
        language => 'en',
    );

Create a Lucy::Simple object, which can be used for both indexing and
searching.  Two hash-style parameters are required.

=over 

=item *

B<path> - Where the index directory should be located.  If no index is found
at the specified location, one will be created.

=item *

B<language> - The language of the documents in your collection, indicated 
by a two-letter ISO code.  12 languages are supported:

    |-----------------------|
    | Language   | ISO code |
    |-----------------------|
    | Danish     | da       |
    | Dutch      | nl       |
    | English    | en       |
    | Finnish    | fi       |
    | French     | fr       |
    | German     | de       |
    | Italian    | it       |
    | Norwegian  | no       |
    | Portuguese | pt       |
    | Spanish    | es       |
    | Swedish    | sv       |
    | Russian    | ru       |
    |-----------------------|

=back

=head2 add_doc 

    $lucy->add_doc({
        location => $url,
        title    => $title,
        content  => $content,
    });

Add a document to the index.  The document must be supplied as a hashref, with
field names as keys and content as values.

=head2 search

    my $total_hits = $lucy->search( 
        query      => $query_string,    # required
        offset     => 40,               # default 0
        num_wanted => 20,               # default 10
    );

Search the index.  Returns the total number of documents which match the
query.  (This number is unlikely to match C<num_wanted>.)

=over

=item *

B<query> - A search query string.

=item *

B<offset> - The number of most-relevant hits to discard, typically used when
"paging" through hits N at a time.  Setting offset to 20 and num_wanted to 10
retrieves hits 21-30, assuming that 30 hits can be found.

=item *

B<num_wanted> - The number of hits you would like to see after C<offset> is
taken into account.  

=back

=head1 BUGS

Not thread-safe.

=cut
