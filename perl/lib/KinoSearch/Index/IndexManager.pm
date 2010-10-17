package KinoSearch::Index::IndexManager;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    use Sys::Hostname qw( hostname );
    my $hostname = hostname() or die "Can't get unique hostname";
    my $manager = KinoSearch::Index::IndexManager->new( 
        host => $hostname,
    );

    # Index time:
    my $indexer = KinoSearch::Index::Indexer->new(
        index => '/path/to/index',
        manager => $manager,
    );

    # Search time:
    my $reader = KinoSearch::Index::IndexReader->open(
        index   => '/path/to/index',
        manager => $manager,
    );
    my $searcher = KinoSearch::Search::IndexSearcher->new( index => $reader );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $manager = KinoSearch::Index::IndexManager->new(
        host => $hostname,    # default: ""
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::IndexManager",
    bind_constructors => ["new"],
    bind_methods      => [
        qw(
            Recycle
            Make_Write_Lock
            Make_Deletion_Lock
            Make_Merge_Lock
            Make_Snapshot_Read_Lock
            Highest_Seg_Num
            Make_Snapshot_Filename
            Set_Folder
            Get_Folder
            Get_Host
            Set_Write_Lock_Timeout
            Get_Write_Lock_Timeout
            Set_Write_Lock_Interval
            Get_Write_Lock_Interval
            Set_Merge_Lock_Timeout
            Get_Merge_Lock_Timeout
            Set_Merge_Lock_Interval
            Get_Merge_Lock_Interval
            Set_Deletion_Lock_Timeout
            Get_Deletion_Lock_Timeout
            Set_Deletion_Lock_Interval
            Get_Deletion_Lock_Interval
            )
    ],
    make_pod => {
        methods => [
            qw(
                make_write_lock
                recycle
                set_folder
                get_folder
                get_host
                set_write_lock_timeout
                get_write_lock_timeout
                set_write_lock_interval
                get_write_lock_interval
                )
        ],
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    },
);


