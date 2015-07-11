# Step-by-step introduction to Apache Lucy.

Explore Apache Lucy's basic functionality by starting with a minimalist CGI
search app based on Lucy::Simple and transforming it, step by step,
into an "advanced search" interface utilizing more flexible core modules like
[](cfish:lucy.Indexer) and [](cfish:lucy.IndexSearcher).

## Chapters

* [](cfish:SimpleTutorial) - Build a bare-bones search app using
  Lucy::Simple.

* [](cfish:BeyondSimpleTutorial) - Rebuild the app using core
  classes like [](cfish:lucy.Indexer) and
  [](cfish:lucy.IndexSearcher) in place of Lucy::Simple.

* [](cfish:FieldTypeTutorial) - Experiment with different field
  characteristics using subclasses of [](cfish:lucy.FieldType).

* [](cfish:AnalysisTutorial) - Examine how the choice of
  [](cfish:lucy.Analyzer) subclass affects search results.

* [](cfish:HighlighterTutorial) - Augment search results with
  highlighted excerpts.

* [](cfish:QueryObjectsTutorial) - Unlock advanced search features
  by using Query objects instead of query strings.

## Source materials

The source material used by the tutorial app -- a multi-text-file presentation
of the United States constitution -- can be found in the `sample` directory
at the root of the Lucy distribution, along with finished indexing and search
apps.

~~~ perl
sample/indexer.pl        # indexing app
sample/search.cgi        # search app
sample/us_constitution   # corpus
~~~

## Conventions

The user is expected to be familiar with OO Perl and basic CGI programming.

The code in this tutorial assumes a Unix-flavored operating system and the
Apache webserver, but will work with minor modifications on other setups.

## See also

More advanced and esoteric subjects are covered in [](cfish:Cookbook).


