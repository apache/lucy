use strict;
use warnings;
use lib 'buildlib';
use constant HAS_LEAKTRACE => eval { require Test::LeakTrace };
use constant USE_LEAKTRACE => 0;    # set to 1 to enable this test
use Test::More ( HAS_LEAKTRACE && USE_LEAKTRACE )
    ? ( tests => 1 )
    : ( skip_all => 'require Test::LeakTrace' );
use Test::LeakTrace;
use KinoSearch::Test;

leaks_cmp_ok {
    my $folder = KinoSearch::Store::RAMFolder->new;
    my $schema = KinoSearch::Test::TestSchema->new;

    my $indexer = KinoSearch::Indexer->new(
        index  => $folder,
        schema => $schema,
    );

    $indexer->add_doc( { content => 'foo' } );
    $indexer->optimize;

}
'<', 1;
