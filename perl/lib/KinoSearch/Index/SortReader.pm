package KinoSearch::Index::SortReader;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::SortReader",
    bind_constructors => ["new"],
    bind_methods      => [qw( Fetch_Sort_Cache )],
);
Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::DefaultSortReader",
    bind_constructors => ["new"],
);


