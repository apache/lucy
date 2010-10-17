package KinoSearch::Search::HitQueue;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Search::HitQueue",
    bind_constructors => ["new"],
);


