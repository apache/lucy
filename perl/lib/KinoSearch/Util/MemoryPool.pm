package KinoSearch::Util::MemoryPool;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Util::MemoryPool",
    bind_constructors => ["new"],
);


