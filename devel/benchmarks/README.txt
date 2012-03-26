Indexing Benchmarks

The purpose of this experiment is to test raw indexing speed, using
Reuters-21578, Distribution 1.0 as a test corpus.  As of this writing,
Reuters-21578 is available at: 
    
    http://www.daviddlewis.com/resources/testcollections/reuters21578

The corpus comes packaged in SGML, which means we need to preprocess it so
that our results are not infected by differences between SGML parsers.  A
simple perl script, "./extract_reuters.plx" is supplied, which expands the
Reuters articles out into the file system, 1 article per file, with the title
as the first line of text.  It takes one command line argument: the location
of the un-tarred Reuters collection.

    ./extract_reuters.plx /path/to/reuters_collection

Filepaths are hard-coded, and the assumption is that the apps will be run from
within the benchmarks/ directory.  Each of the indexing apps takes four
optional command line arguments: 

  * The number of documents to index.
  * The number of times to repeat the indexing process.
  * The increment, or number of docs to add during each index writer instance.
  * Whether or not the main text should be stored and highlightable.

    $ perl -Mblib indexers/lucy_indexer.plx \
    > --docs=1000 --reps=6 --increment=10 --store=1

    $ java -server -Xmx500M -XX:CompileThreshold=100 LuceneIndexer \
    > -docs 1000 -reps 6 -increment 10 -store 1

If no command line args are supplied, the apps will index the entire 19043
article collection once, using a single index writer, and will neither store
nor vectorize the main text.

Upon finishing, each app will produce a "truncated mean" report: the slowest
25% and fastest 25% of  reps will be discarded, and the rest will be averaged. 

