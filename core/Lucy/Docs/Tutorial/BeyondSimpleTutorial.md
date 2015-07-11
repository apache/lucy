# A more flexible app structure.

## Goal

In this tutorial chapter, we'll refactor the apps we built in
[](cfish:SimpleTutorial) so that they look exactly the same from
the end user's point of view, but offer the developer greater possibilites for
expansion.  

To achieve this, we'll ditch Lucy::Simple and replace it with the
classes that it uses internally:

* [](cfish:lucy.Schema) - Plan out your index.
* [](cfish:lucy.FullTextType) - Field type for full text search.
* [](cfish:lucy.EasyAnalyzer) - A one-size-fits-all parser/tokenizer.
* [](cfish:lucy.Indexer) - Manipulate index content.
* [](cfish:lucy.IndexSearcher) - Search an index.
* [](cfish:lucy.Hits) - Iterate over hits returned by a Searcher.

## Adaptations to indexer.pl

After we load our modules...

~~~ perl
use Lucy::Plan::Schema;
use Lucy::Plan::FullTextType;
use Lucy::Analysis::EasyAnalyzer;
use Lucy::Index::Indexer;
~~~

... the first item we're going need is a [](cfish:lucy.Schema).

The primary job of a Schema is to specify what fields are available and how
they're defined.  We'll start off with three fields: title, content and url.

~~~ perl
# Create Schema.
my $schema = Lucy::Plan::Schema->new;
my $easyanalyzer = Lucy::Analysis::EasyAnalyzer->new(
    language => 'en',
);
my $type = Lucy::Plan::FullTextType->new(
    analyzer => $easyanalyzer,
);
$schema->spec_field( name => 'title',   type => $type );
$schema->spec_field( name => 'content', type => $type );
$schema->spec_field( name => 'url',     type => $type );
~~~

All of the fields are spec'd out using the "FullTextType" FieldType,
indicating that they will be searchable as "full text" -- which means that
they can be searched for individual words.  The "analyzer", which is unique to
FullTextType fields, is what breaks up the text into searchable tokens.

Next, we'll swap our Lucy::Simple object out for a Lucy::Index::Indexer.
The substitution will be straightforward because Simple has merely been
serving as a thin wrapper around an inner Indexer, and we'll just be peeling
away the wrapper.

First, replace the constructor:

~~~ perl
# Create Indexer.
my $indexer = Lucy::Index::Indexer->new(
    index    => $path_to_index,
    schema   => $schema,
    create   => 1,
    truncate => 1,
);
~~~

Next, have the `$indexer` object `add_doc` where we were having the
`$lucy` object `add_doc` before:

~~~ perl
foreach my $filename (@filenames) {
    my $doc = parse_file($filename);
    $indexer->add_doc($doc);
}
~~~

There's only one extra step required: at the end of the app, you must call
commit() explicitly to close the indexing session and commit your changes.
(Lucy::Simple hides this detail, calling commit() implicitly when it needs to).

~~~ perl
$indexer->commit;
~~~

## Adaptations to search.cgi

In our search app as in our indexing app, Lucy::Simple has served as a
thin wrapper -- this time around [](cfish:lucy.IndexSearcher) and
[](cfish:lucy.Hits).  Swapping out Simple for these two classes is
also straightforward:

~~~ perl
use Lucy::Search::IndexSearcher;

my $searcher = Lucy::Search::IndexSearcher->new( 
    index => $path_to_index,
);
my $hits = $searcher->hits(    # returns a Hits object, not a hit count
    query      => $q,
    offset     => $offset,
    num_wanted => $page_size,
);
my $hit_count = $hits->total_hits;  # get the hit count here

...

while ( my $hit = $hits->next ) {
    ...
}
~~~

## Hooray!

Congratulations!  Your apps do the same thing as before... but now they'll be
easier to customize.  

In our next chapter, ()[cfish:FieldTypeTutorial), we'll explore
how to assign different behaviors to different fields.


