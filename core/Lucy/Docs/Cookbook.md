# Apache Lucy recipes

The Cookbook provides thematic documentation covering some of Apache Lucy's
more sophisticated features.  For a step-by-step introduction to Lucy,
see [](cfish:Tutorial).

## Chapters

* [](cfish:FastUpdates) - While index updates are fast on
  average, worst-case update performance may be significantly slower. To make
  index updates consistently quick, we must manually intervene to control the
  process of index segment consolidation.

* [](cfish:CustomQuery) - Explore Lucy's support for
  custom query types by creating a "PrefixQuery" class to handle trailing
  wildcards.

* [](cfish:CustomQueryParser) - Define your own custom
  search query syntax using [](cfish:lucy.QueryParser) and
  Parse::RecDescent.

## Materials

Some of the recipes in the Cookbook reference the completed
[](cfish:Tutorial) application.  These materials can be
found in the `sample` directory at the root of the Lucy distribution:

~~~ perl
sample/indexer.pl        # indexing app
sample/search.cgi        # search app
sample/us_constitution   # corpus
~~~

