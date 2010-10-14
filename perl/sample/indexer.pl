#!/usr/local/bin/perl
use strict;
use warnings;

# (Change configuration variables as needed.)
my $path_to_index = '/path/to/index';
my $uscon_source  = '/usr/local/apache2/htdocs/us_constitution';

use File::Spec::Functions qw( catfile );
use HTML::TreeBuilder;
use KinoSearch::Plan::Schema;
use KinoSearch::Plan::FullTextType;
use KinoSearch::Analysis::PolyAnalyzer;
use KinoSearch::Index::Indexer;

# Create Schema.
my $schema = KinoSearch::Plan::Schema->new;
my $polyanalyzer = KinoSearch::Analysis::PolyAnalyzer->new(
    language => 'en',
);
my $title_type = KinoSearch::Plan::FullTextType->new( 
    analyzer => $polyanalyzer, 
);
my $content_type = KinoSearch::Plan::FullTextType->new(
    analyzer      => $polyanalyzer,
    highlightable => 1,
);
my $url_type = KinoSearch::Plan::StringType->new( indexed => 0, );
my $cat_type = KinoSearch::Plan::StringType->new( stored => 0, );
$schema->spec_field( name => 'title',    type => $title_type );
$schema->spec_field( name => 'content',  type => $content_type );
$schema->spec_field( name => 'url',      type => $url_type );
$schema->spec_field( name => 'category', type => $cat_type );

# Create an Indexer object.
my $indexer = KinoSearch::Index::Indexer->new(
    index    => $path_to_index,
    schema   => $schema,
    create   => 1,
    truncate => 1,
);

# Collect names of source html files.
opendir( my $dh, $uscon_source )
    or die "Couldn't opendir '$uscon_source': $!";
my @filenames = grep { $_ =~ /\.html/ && $_ ne 'index.html' } readdir $dh;

# Iterate over list of source files.
for my $filename (@filenames) {
    print "Indexing $filename\n";
    my $doc = parse_file($filename);
    $indexer->add_doc($doc);
}

# Finalize the index and print a confirmation message.
$indexer->commit;
print "Finished.\n";

# Parse an HTML file from our US Constitution collection and return a
# hashref with the fields title, body, url, and category.
sub parse_file {
    my $filename = shift;
    my $filepath = catfile( $uscon_source, $filename );
    my $tree     = HTML::TreeBuilder->new;
    $tree->parse_file($filepath);
    my $title_node = $tree->look_down( _tag => 'title' )
        or die "No title element in $filepath";
    my $bodytext_node = $tree->look_down( id => 'bodytext' )
        or die "No div with id 'bodytext' in $filepath";
    my $category
        = $filename =~ /art/      ? 'article'
        : $filename =~ /amend/    ? 'amendment'
        : $filename =~ /preamble/ ? 'preamble'
        :                           die "Can't derive category for $filename";
    return {
        title    => $title_node->as_trimmed_text,
        content  => $bodytext_node->as_trimmed_text,
        url      => "/us_constitution/$filename",
        category => $category,
    };
}
