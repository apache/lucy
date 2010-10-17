package KinoSearch::Object::LockFreeRegistry;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Object::LockFreeRegistry",
    bind_methods      => [qw( Register Fetch )],
    bind_constructors => ["new"],
);


