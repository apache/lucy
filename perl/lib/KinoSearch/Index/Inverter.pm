package KinoSearch::Index::Inverter;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::Inverter",
    bind_constructors => ["new"],
    bind_methods      => [
        qw(
            Get_Doc
            Iterate
            Next
            Clear
            Get_Field_Name
            Get_Value
            Get_Type
            Get_Analyzer
            Get_Similarity
            Get_Inversion
            )
    ],
);


