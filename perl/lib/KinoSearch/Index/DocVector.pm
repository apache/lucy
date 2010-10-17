package KinoSearch::Index::DocVector;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::DocVector",
    bind_methods      => [qw( Term_Vector Field_Buf Add_Field_Buf )],
    bind_constructors => ["new"],
);


