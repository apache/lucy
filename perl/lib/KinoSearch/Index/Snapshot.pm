package KinoSearch::Index::Snapshot;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $snapshot = KinoSearch::Index::Snapshot->new;
    $snapshot->read_file( folder => $folder );    # load most recent snapshot
    my $files = $snapshot->list;
    print "$_\n" for @$files;
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $snapshot = KinoSearch::Index::Snapshot->new;
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Index::Snapshot",
    bind_methods => [
        qw(
            List
            Num_Entries
            Add_Entry
            Delete_Entry
            Read_File
            Write_File
            Set_Path
            Get_Path
            )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
        methods     => [
            qw(
                list
                num_entries
                add_entry
                delete_entry
                read_file
                write_file
                set_path
                get_path
                )
        ],
    },
);


