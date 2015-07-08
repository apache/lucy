# Characteristics of Apache Lucy document ids.

## Document ids are signed 32-bit integers

Document ids in Apache Lucy start at 1.  Because 0 is never a valid doc id, we
can use it as a sentinel value:

~~~ perl
while ( my $doc_id = $posting_list->next ) {
    ...
}
~~~

## Document ids are ephemeral

The document ids used by Lucy are associated with a single index
snapshot.  The moment an index is updated, the mapping of document ids to
documents is subject to change.

Since IndexReader objects represent a point-in-time view of an index, document
ids are guaranteed to remain static for the life of the reader.  However,
because they are not permanent, Lucy document ids cannot be used as
foreign keys to locate records in external data sources.  If you truly need a
primary key field, you must define it and populate it yourself.

Furthermore, the order of document ids does not tell you anything about the
sequence in which documents were added to the index.

