package KinoSearch::Index::SegWriter;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::SegWriter",
    bind_constructors => ["new"],
    bind_methods      => [
        qw(
            Add_Writer
            Register
            Fetch
            )
    ],
    make_pod => {
        methods => [
            qw(
                add_doc
                add_writer
                register
                fetch
                )
        ],
    }
);


