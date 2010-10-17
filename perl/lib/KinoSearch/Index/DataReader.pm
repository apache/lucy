package KinoSearch::Index::DataReader;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    # Abstract base class.
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $reader = MyDataReader->new(
        schema   => $seg_reader->get_schema,      # default undef
        folder   => $seg_reader->get_folder,      # default undef
        snapshot => $seg_reader->get_snapshot,    # default undef
        segments => $seg_reader->get_segments,    # default undef
        seg_tick => $seg_reader->get_seg_tick,    # default -1
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Index::DataReader",
    bind_methods => [
        qw(
            Get_Schema
            Get_Folder
            Get_Segments
            Get_Snapshot
            Get_Seg_Tick
            Get_Segment
            Aggregator
            Close
            )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor, },
        methods     => [
            qw(
                get_schema
                get_folder
                get_snapshot
                get_segments
                get_segment
                get_seg_tick
                aggregator
                )
        ]
    },
);


