package KinoSearch::Index::DeletionsWriter;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $polyreader  = $del_writer->get_polyreader;
    my $seg_readers = $polyreader->seg_readers;
    for my $seg_reader (@$seg_readers) {
        my $count = $del_writer->seg_del_count( $seg_reader->get_seg_name );
        ...
    }
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Index::DeletionsWriter",
    bind_methods => [
        qw(
            Generate_Doc_Map
            Delete_By_Term
            Delete_By_Query
            Delete_By_Doc_ID
            Updated
            Seg_Deletions
            Seg_Del_Count
            )
    ],
    make_pod => {
        synopsis => $synopsis,
        methods  => [
            qw(
                Delete_By_Term
                Delete_By_Query
                Updated
                Seg_Del_Count
                )
        ],
    },
);
Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::DefaultDeletionsWriter",
    bind_constructors => ["new"],
);


