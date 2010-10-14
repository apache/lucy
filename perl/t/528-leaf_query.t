use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 12;
use Storable qw( freeze thaw );
use KinoSearch::Test::TestUtils qw( create_index );

my $folder = create_index( 'a', 'b', 'b c', 'c', 'c d', 'd', 'e' );
my $searcher = KinoSearch::Search::IndexSearcher->new( index => $folder );
my $reader = $searcher->get_reader;

my $leaf_query = KinoSearch::Search::LeafQuery->new(
    field => 'content',
    text  => 'b'
);
my $no_field_leaf_query = KinoSearch::Search::LeafQuery->new( text => 'b' );
is( $leaf_query->to_string,          "content:b", "to_string" );
is( $no_field_leaf_query->to_string, "b",         "no field to_string" );

is( $leaf_query->get_field, "content", "get field" );
ok( !defined $no_field_leaf_query->get_field, "get null field" );
is( $leaf_query->get_text, 'b', 'get text' );

ok( !$leaf_query->equals($no_field_leaf_query), "!equals (field/nofield)" );
ok( !$no_field_leaf_query->equals($leaf_query), "!equals (nofield/field)" );

my $diff_field
    = KinoSearch::Search::LeafQuery->new( field => 'oink', text => 'b' );
ok( !$diff_field->equals($leaf_query), "!equals (different field)" );

my $diff_text
    = KinoSearch::Search::LeafQuery->new( field => 'content', text => 'c' );
ok( !$diff_text->equals($leaf_query), "!equals (different text)" );

eval { $leaf_query->make_compiler( searcher => $searcher ); };
like( $@, qr/Make_Compiler/, "Make_Compiler throws error" );

my $frozen = freeze($leaf_query);
my $thawed = thaw($frozen);
ok( $leaf_query->equals($thawed), "freeze/thaw and equals" );
$leaf_query->set_boost(2);
ok( !$leaf_query->equals($thawed), "!equals (boost)" );

