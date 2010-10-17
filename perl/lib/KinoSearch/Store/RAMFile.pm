package KinoSearch::Store::RAMFile;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Store::RAMFile",
    bind_methods      => [qw( Get_Contents )],
    bind_constructors => ['new'],
);


