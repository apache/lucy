use strict;
use warnings;

package MySchema;
use base qw( KinoSearch::Plan::Schema );
use KinoSearch::Analysis::Tokenizer;

sub new {
    my $self = shift->SUPER::new(@_);
    my $type = KinoSearch::Plan::FullTextType->new(
        analyzer => KinoSearch::Analysis::Tokenizer->new, );
    $self->spec_field( name => 'title', type => $type );
    $self->spec_field( name => 'body',  type => $type );
    return $self;
}

package main;

use Test::More tests => 12;
use KinoSearch::Test;

my $folder  = KinoSearch::Store::RAMFolder->new;
my $schema  = MySchema->new;
my $indexer = KinoSearch::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
);
my %docs = (
    'a' => 'foo',
    'b' => 'bar',
);

while ( my ( $title, $body ) = each %docs ) {
    $indexer->add_doc(
        {   title => $title,
            body  => $body,
        }
    );
}
$indexer->commit;

my $searcher = KinoSearch::Search::IndexSearcher->new( index => $folder );

my $tokenizer = KinoSearch::Analysis::Tokenizer->new;
my $or_parser = KinoSearch::Search::QueryParser->new(
    schema   => $schema,
    analyzer => $tokenizer,
    fields   => [ 'title', 'body', ],
);
my $and_parser = KinoSearch::Search::QueryParser->new(
    schema         => $schema,
    analyzer       => $tokenizer,
    fields         => [ 'title', 'body', ],
    default_boolop => 'AND',
);

sub test_qstring {
    my ( $qstring, $expected, $message ) = @_;

    my $hits = $searcher->hits( query => $qstring );
    is( $hits->total_hits, $expected, $message );

    my $query = $or_parser->parse($qstring);
    $hits = $searcher->hits( query => $query );
    is( $hits->total_hits, $expected, "OR: $message" );

    $query = $and_parser->parse($qstring);
    $hits = $searcher->hits( query => $query );
    is( $hits->total_hits, $expected, "AND: $message" );
}

test_qstring( 'a foo', 1, "simple match across multiple fields" );
test_qstring( 'a -foo', 0,
    "match of negated term on any field should exclude document" );
test_qstring(
    'a +foo',
    1,
    "failure to match of required term on a field "
        . "should not exclude doc if another field matches."
);
test_qstring( '+a +foo', 1,
    "required terms spread across disparate fields should match" );
