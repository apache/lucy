package KinoSearch::Index::SegLexicon;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::SegLexicon",
    bind_methods      => [qw( Get_Term_Info Get_Field_Num )],
    bind_constructors => ["new"],
);


