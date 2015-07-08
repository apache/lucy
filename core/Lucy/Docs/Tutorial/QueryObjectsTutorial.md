# Use Query objects instead of query strings.

Until now, our search app has had only a single search box.  In this tutorial
chapter, we'll move towards an "advanced search" interface, by adding a
"category" drop-down menu.  Three new classes will be required:

* [](cfish:lucy.QueryParser) - Turn a query string into a
  [](cfish:lucy.Query) object.

* [](cfish:lucy.TermQuery) - Query for a specific term within
  a specific field.

* [](cfish:lucy.ANDQuery) - "AND" together multiple Query
objects to produce an intersected result set.

## Adaptations to indexer.pl

Our new "category" field will be a StringType field rather than a FullTextType
field, because we will only be looking for exact matches.  It needs to be
indexed, but since we won't display its value, it doesn't need to be stored.

~~~ perl
my $cat_type = Lucy::Plan::StringType->new( stored => 0 );
$schema->spec_field( name => 'category', type => $cat_type );
~~~

There will be three possible values: "article", "amendment", and "preamble",
which we'll hack out of the source file's name during our `parse_file`
subroutine:

~~~ perl
my $category
    = $filename =~ /art/      ? 'article'
    : $filename =~ /amend/    ? 'amendment'
    : $filename =~ /preamble/ ? 'preamble'
    :                           die "Can't derive category for $filename";
return {
    title    => $title,
    content  => $bodytext,
    url      => "/us_constitution/$filename",
    category => $category,
};
~~~

## Adaptations to search.cgi

The "category" constraint will be added to our search interface using an HTML
"select" element (this routine will need to be integrated into the HTML
generation section of search.cgi):

~~~ perl
# Build up the HTML "select" object for the "category" field.
sub generate_category_select {
    my $cat = shift;
    my $select = qq|
      <select name="category">
        <option value="">All Sections</option>
        <option value="article">Articles</option>
        <option value="amendment">Amendments</option>
      </select>|;
    if ($cat) {
        $select =~ s/"$cat"/"$cat" selected/;
    }
    return $select;
}
~~~

We'll start off by loading our new modules and extracting our new CGI
parameter.

~~~ perl
use Lucy::Search::QueryParser;
use Lucy::Search::TermQuery;
use Lucy::Search::ANDQuery;

... 

my $category = decode( "UTF-8", $cgi->param('category') || '' );
~~~

QueryParser's constructor requires a "schema" argument.  We can get that from
our IndexSearcher:

~~~ perl
# Create an IndexSearcher and a QueryParser.
my $searcher = Lucy::Search::IndexSearcher->new( 
    index => $path_to_index, 
);
my $qparser  = Lucy::Search::QueryParser->new( 
    schema => $searcher->get_schema,
);
~~~

Previously, we have been handing raw query strings to IndexSearcher.  Behind
the scenes, IndexSearcher has been using a QueryParser to turn those query
strings into Query objects.  Now, we will bring QueryParser into the
foreground and parse the strings explicitly.

~~~ perl
my $query = $qparser->parse($q);
~~~

If the user has specified a category, we'll use an ANDQuery to join our parsed
query together with a TermQuery representing the category.

~~~ perl
if ($category) {
    my $category_query = Lucy::Search::TermQuery->new(
        field => 'category', 
        term  => $category,
    );
    $query = Lucy::Search::ANDQuery->new(
        children => [ $query, $category_query ]
    );
}
~~~

Now when we execute the query...

~~~ perl
# Execute the Query and get a Hits object.
my $hits = $searcher->hits(
    query      => $query,
    offset     => $offset,
    num_wanted => $page_size,
);
~~~

... we'll get a result set which is the intersection of the parsed query and
the category query.

## Using TermQuery with full text fields

When querying full text fields, the easiest way is to create query objects
using QueryParser. But sometimes you want to create TermQuery for a single
term in a FullTextType field directly. In this case, we have to run the
search term through the field's analyzer to make sure it gets normalized in
the same way as the field's content.

~~~ perl
sub make_term_query {
    my ($field, $term) = @_;

    my $token;
    my $type = $schema->fetch_type($field);

    if ( $type->isa('Lucy::Plan::FullTextType') ) {
        # Run the term through the full text analysis chain.
        my $analyzer = $type->get_analyzer;
        my $tokens   = $analyzer->split($term);

        if ( @$tokens != 1 ) {
            # If the term expands to more than one token, or no
            # tokens at all, it will never match a token in the
            # full text field.
            return Lucy::Search::NoMatchQuery->new;
        }

        $token = $tokens->[0];
    }
    else {
        # Exact match for other types.
        $token = $term;
    }

    return Lucy::Search::TermQuery->new(
        field => $field,
        term  => $token,
    );
}
~~~

## Congratulations!

You've made it to the end of the tutorial.

## See Also

For additional thematic documentation, see the Apache Lucy
[](cfish:Cookbook).

ANDQuery has a companion class, [](cfish:lucy.ORQuery), and a
close relative, [](cfish:lucy.RequiredOptionalQuery).


