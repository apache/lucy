# How to choose and use Analyzers.

Try swapping out the EasyAnalyzer in our Schema for a StandardTokenizer:

~~~ perl
my $tokenizer = Lucy::Analysis::StandardTokenizer->new;
my $type = Lucy::Plan::FullTextType->new(
    analyzer => $tokenizer,
);
~~~

Search for `senate`, `Senate`, and `Senator` before and after making the
change and re-indexing.

Under EasyAnalyzer, the results are identical for all three searches, but
under StandardTokenizer, searches are case-sensitive, and the result sets for
`Senate` and `Senator` are distinct.

## EasyAnalyzer

What's happening is that EasyAnalyzer is performing more aggressive processing
than StandardTokenizer.  In addition to tokenizing, it's also converting all
text to lower case so that searches are case-insensitive, and using a
"stemming" algorithm to reduce related words to a common stem (`senat`, in
this case).

EasyAnalyzer is actually multiple Analyzers wrapped up in a single package.
In this case, it's three-in-one, since specifying a EasyAnalyzer with
`language => 'en'` is equivalent to this snippet:

~~~ perl
my $tokenizer    = Lucy::Analysis::StandardTokenizer->new;
my $normalizer   = Lucy::Analysis::Normalizer->new;
my $stemmer      = Lucy::Analysis::SnowballStemmer->new( language => 'en' );
my $polyanalyzer = Lucy::Analysis::PolyAnalyzer->new(
    analyzers => [ $tokenizer, $normalizer, $stemmer ],
);
~~~

You can add or subtract Analyzers from there if you like.  Try adding a fourth
Analyzer, a SnowballStopFilter for suppressing "stopwords" like "the", "if",
and "maybe".

~~~ perl
my $stopfilter = Lucy::Analysis::SnowballStopFilter->new( 
    language => 'en',
);
my $polyanalyzer = Lucy::Analysis::PolyAnalyzer->new(
    analyzers => [ $tokenizer, $normalizer, $stopfilter, $stemmer ],
);
~~~

Also, try removing the SnowballStemmer.

~~~ perl
my $polyanalyzer = Lucy::Analysis::PolyAnalyzer->new(
    analyzers => [ $tokenizer, $normalizer ],
);
~~~

The original choice of a stock English EasyAnalyzer probably still yields the
best results for this document collection, but you get the idea: sometimes you
want a different Analyzer.

## When the best Analyzer is no Analyzer

Sometimes you don't want an Analyzer at all.  That was true for our "url"
field because we didn't need it to be searchable, but it's also true for
certain types of searchable fields.  For instance, "category" fields are often
set up to match exactly or not at all, as are fields like "last_name" (because
you may not want to conflate results for "Humphrey" and "Humphries").

To specify that there should be no analysis performed at all, use StringType:

~~~ perl
my $type = Lucy::Plan::StringType->new;
$schema->spec_field( name => 'category', type => $type );
~~~

## Highlighting up next

In our next tutorial chapter, [](cfish:HighlighterTutorial),
we'll add highlighted excerpts from the "content" field to our search results.


