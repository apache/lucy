package KinoSearch::Search::PolyQuery;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Search::PolyQuery",
    bind_methods      => [qw( Add_Child Set_Children Get_Children )],
    bind_constructors => ["new"],
    make_pod          => {}
);


