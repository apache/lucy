use strict;
use warnings;
use Test::More tests => 4;
use KinoSearch::Test;

my $folder   = KinoSearch::Store::RAMFolder->new;
my $snapshot = KinoSearch::Index::Snapshot->new;
$snapshot->add_entry("foo");
$snapshot->add_entry("bar");
ok( $snapshot->delete_entry("bar"), "delete_entry" );
is_deeply( $snapshot->list, ['foo'], "add_entry, list" );
$snapshot->write_file( folder => $folder );
is( $snapshot->read_file( folder => $folder ),
    $snapshot, "write_file, read_file" );
$snapshot->set_path("snapfile");
is( $snapshot->get_path, "snapfile", "set_path, get_path" );

