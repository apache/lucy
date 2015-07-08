# Augment search results with highlighted excerpts.

Adding relevant excerpts with highlighted search terms to your search results
display makes it much easier for end users to scan the page and assess which
hits look promising, dramatically improving their search experience.

## Adaptations to indexer.pl

[](cfish:lucy.Highlighter) uses information generated at index
time.  To save resources, highlighting is disabled by default and must be
turned on for individual fields.

~~~ perl
my $highlightable = Lucy::Plan::FullTextType->new(
    analyzer      => $polyanalyzer,
    highlightable => 1,
);
$schema->spec_field( name => 'content', type => $highlightable );
~~~

## Adaptations to search.cgi

To add highlighting and excerpting to the search.cgi sample app, create a
`$highlighter` object outside the hits iterating loop...

~~~ perl
my $highlighter = Lucy::Highlight::Highlighter->new(
    searcher => $searcher,
    query    => $q,
    field    => 'content'
);
~~~

... then modify the loop and the per-hit display to generate and include the
excerpt.

~~~ perl
# Create result list.
my $report = '';
while ( my $hit = $hits->next ) {
    my $score   = sprintf( "%0.3f", $hit->get_score );
    my $excerpt = $highlighter->create_excerpt($hit);
    $report .= qq|
        <p>
          <a href="$hit->{url}"><strong>$hit->{title}</strong></a>
          <em>$score</em>
          <br />
          $excerpt
          <br />
          <span class="excerptURL">$hit->{url}</span>
        </p>
    |;
}
~~~

## Next chapter: Query objects

Our next tutorial chapter, [](cfish:QueryObjectsTutorial),
illustrates how to build an "advanced search" interface using
[](cfish:lucy.Query) objects instead of query strings.


