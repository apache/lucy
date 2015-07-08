# Near real-time index updates

While index updates are fast on average, worst-case update performance may be
significantly slower.  To make index updates consistently quick, we must
manually intervene to control the process of index segment consolidation.

## The problem

Ordinarily, modifying an index is cheap. New data is added to new segments,
and the time to write a new segment scales more or less linearly with the
number of documents added during the indexing session.  

Deletions are also cheap most of the time, because we don't remove documents
immediately but instead mark them as deleted, and adding the deletion mark is
cheap.

However, as new segments are added and the deletion rate for existing segments
increases, search-time performance slowly begins to degrade.  At some point,
it becomes necessary to consolidate existing segments, rewriting their data
into a new segment.  

If the recycled segments are small, the time it takes to rewrite them may not
be significant.  Every once in a while, though, a large amount of data must be
rewritten.

## Procrastinating and playing catch-up

The simplest way to force fast index updates is to avoid rewriting anything.

Indexer relies upon [](cfish:lucy.IndexManager)'s
recycle() method to tell it which segments should be consolidated.  If we
subclass IndexManager and override recycle() so that it always returns an
empty array, we get consistently quick performance:

~~~ perl
package NoMergeManager;
use base qw( Lucy::Index::IndexManager );
sub recycle { [] }

package main;
my $indexer = Lucy::Index::Indexer->new(
    index => '/path/to/index',
    manager => NoMergeManager->new,
);
...
$indexer->commit;
~~~

However, we can't procrastinate forever.  Eventually, we'll have to run an
ordinary, uncontrolled indexing session, potentially triggering a large
rewrite of lots of small and/or degraded segments:

~~~ perl
my $indexer = Lucy::Index::Indexer->new( 
    index => '/path/to/index', 
    # manager => NoMergeManager->new,
);
...
$indexer->commit;
~~~

## Acceptable worst-case update time, slower degradation

Never merging anything at all in the main indexing process is probably
overkill.  Small segments are relatively cheap to merge; we just need to guard
against the big rewrites.  

Setting a ceiling on the number of documents in the segments to be recycled
allows us to avoid a mass proliferation of tiny, single-document segments,
while still offering decent worst-case update speed:

~~~ perl
package LightMergeManager;
use base qw( Lucy::Index::IndexManager );

sub recycle {
    my $self = shift;
    my $seg_readers = $self->SUPER::recycle(@_);
    @$seg_readers = grep { $_->doc_max < 10 } @$seg_readers;
    return $seg_readers;
}
~~~

However, we still have to consolidate every once in a while, and while that
happens content updates will be locked out.

## Background merging

If it's not acceptable to lock out updates while the index consolidation
process runs, the alternative is to move the consolidation process out of
band, using Lucy::Index::BackgroundMerger.  

It's never safe to have more than one Indexer attempting to modify the content
of an index at the same time, but a BackgroundMerger and an Indexer can
operate simultaneously:

~~~ perl
# Indexing process.
use Scalar::Util qw( blessed );
my $retries = 0;
while (1) {
    eval {
        my $indexer = Lucy::Index::Indexer->new(
                index => '/path/to/index',
                manager => LightMergeManager->new,
            );
        $indexer->add_doc($doc);
        $indexer->commit;
    };
    last unless $@;
    if ( blessed($@) and $@->isa("Lucy::Store::LockErr") ) {
        # Catch LockErr.
        warn "Couldn't get lock ($retries retries)";
        $retries++;
    }
    else {
        die "Write failed: $@";
    }
}

# Background merge process.
my $manager = Lucy::Index::IndexManager->new;
$manager->set_write_lock_timeout(60_000);
my $bg_merger = Lucy::Index::BackgroundMerger->new(
    index   => '/path/to/index',
    manager => $manager,
);
$bg_merger->commit;
~~~

The exception handling code becomes useful once you have more than one index
modification process happening simultaneously.  By default, Indexer tries
several times to acquire a write lock over the span of one second, then holds
it until commit() completes.  BackgroundMerger handles most of its work
without the write lock, but it does need it briefly once at the beginning and
once again near the end.  Under normal loads, the internal retry logic will
resolve conflicts, but if it's not acceptable to miss an insert, you probably
want to catch LockErr exceptions thrown by Indexer.  In contrast, a LockErr
from BackgroundMerger probably just needs to be logged.

