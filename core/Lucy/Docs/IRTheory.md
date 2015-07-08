# Crash course in information retrieval

Just enough Information Retrieval theory to find your way around Apache Lucy.

## Terminology

Lucy uses some terminology from the field of information retrieval which
may be unfamiliar to many users.  "Document" and "term" mean pretty much what
you'd expect them to, but others such as "posting" and "inverted index" need a
formal introduction:

* _document_ - An atomic unit of retrieval.
* _term_ - An attribute which describes a document.
* _posting_ - One term indexing one document.
* _term list_ - The complete list of terms which describe a document.
* _posting list_ - The complete list of documents which a term indexes.
* _inverted index_ - A data structure which maps from terms to documents.

Since Lucy is a practical implementation of IR theory, it loads these
abstract, distilled definitions down with useful traits.  For instance, a
"posting" in its most rarefied form is simply a term-document pairing; in
Lucy, the class [](cfish:lucy.MatchPosting) fills this
role.  However, by associating additional information with a posting like the
number of times the term occurs in the document, we can turn it into a
[](cfish:lucy.ScorePosting), making it possible
to rank documents by relevance rather than just list documents which happen to
match in no particular order.

## TF/IDF ranking algorithm

Lucy uses a variant of the well-established "Term Frequency / Inverse
Document Frequency" weighting scheme.  A thorough treatment of TF/IDF is too
ambitious for our present purposes, but in a nutshell, it means that...

* in a search for `skate park`, documents which score well for the
  comparatively rare term `skate` will rank higher than documents which score
  well for the more common term `park`.

* a 10-word text which has one occurrence each of both `skate` and `park` will
  rank higher than a 1000-word text which also contains one occurrence of each.

A web search for "tf idf" will turn up many excellent explanations of the
algorithm.

