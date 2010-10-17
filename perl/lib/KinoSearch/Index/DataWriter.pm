package KinoSearch::Index::DataWriter;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<END_SYNOPSIS;
    # Abstract base class.
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $writer = MyDataWriter->new(
        snapshot   => $snapshot,      # required
        segment    => $segment,       # required
        polyreader => $polyreader,    # required
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Index::DataWriter",
    bind_methods => [
        qw(
            Add_Inverted_Doc
            Add_Segment
            Delete_Segment
            Merge_Segment
            Finish
            Format
            Metadata
            Get_Snapshot
            Get_Segment
            Get_PolyReader
            Get_Schema
            Get_Folder
            )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
        methods     => [
            qw(
                add_inverted_doc
                add_segment
                delete_segment
                merge_segment
                finish
                format
                metadata
                get_snapshot
                get_segment
                get_polyreader
                get_schema
                get_folder
                )
        ],
    },
);


