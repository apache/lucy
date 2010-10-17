package KinoSearch::Index::TermVector;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::TermVector",
    bind_constructors => ["new"],
    bind_methods      => [
        qw(
            Get_Positions
            Get_Start_Offsets
            Get_End_Offsets
            )
    ],
);


