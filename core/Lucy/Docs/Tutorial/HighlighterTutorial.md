# Augment search results with highlighted excerpts.

Adding relevant excerpts with highlighted search terms to your search results
display makes it much easier for end users to scan the page and assess which
hits look promising, dramatically improving their search experience.

## Adaptations to indexer.pl

[](cfish:lucy.Highlighter) uses information generated at index
time.  To save resources, highlighting is disabled by default and must be
turned on for individual fields.

``` c
    {
        String *field_str = Str_newf("content");
        FullTextType *type = FullTextType_new((Analyzer*)analyzer);
        FullTextType_Set_Highlightable(type, true);
        Schema_Spec_Field(schema, field_str, (FieldType*)type);
        DECREF(type);
        DECREF(field_str);
    }
```

``` perl
my $highlightable = Lucy::Plan::FullTextType->new(
    analyzer      => $easyanalyzer,
    highlightable => 1,
);
$schema->spec_field( name => 'content', type => $highlightable );
```

## Adaptations to search.cgi

To add highlighting and excerpting to the search.cgi sample app, create a
`$highlighter` object outside the hits iterating loop...

``` c
    String *content_str = Str_newf("content");
    Highlighter *highlighter
        = Highlighter_new((Searcher*)searcher, (Obj*)query,
                          content_str, 200);
```

``` perl
my $highlighter = Lucy::Highlight::Highlighter->new(
    searcher => $searcher,
    query    => $q,
    field    => 'content'
);
```

... then modify the loop and the per-hit display to generate and include the
excerpt.

``` c
    String *title_str = Str_newf("title");
    String *url_str   = Str_newf("url");
    HitDoc *hit;
    i = 1;

    // Loop over search results.
    while (NULL != (hit = Hits_Next(hits))) {
        String *title = (String*)HitDoc_Extract(hit, title_str);
        char *title_c = Str_To_Utf8(title);

        String *url = (String*)HitDoc_Extract(hit, url_str);
        char *url_c = Str_To_Utf8(url);

        String *excerpt = Highlighter_Create_Excerpt(highlighter, hit);
        char *excerpt_c = Str_To_Utf8(excerpt);

        printf("Result %d: %s (%s)\n%s\n\n", i, title_c, url_c, excerpt_c);

        free(excerpt_c);
        free(url_c);
        free(title_c);
        DECREF(excerpt);
        DECREF(url);
        DECREF(title);
        DECREF(hit);
        i++;
    }

    DECREF(url_str);
    DECREF(title_str);
    DECREF(hits);
    DECREF(query_str);
    DECREF(highlighter);
    DECREF(content_str);
    DECREF(searcher);
    DECREF(folder);
```

``` perl
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
```

## Next chapter: Query objects

Our next tutorial chapter, [](cfish:QueryObjectsTutorial),
illustrates how to build an "advanced search" interface using
[](cfish:lucy.Query) objects instead of query strings.


