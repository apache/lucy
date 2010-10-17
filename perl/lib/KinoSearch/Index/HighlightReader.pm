package KinoSearch::Index::HighlightReader;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::HighlightReader",
    bind_constructors => ["new"],
);
Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::DefaultHighlightReader",
    bind_constructors => ["new"],
);


