package KinoSearch::Store::RAMFileHandle;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Store::RAMFileHandle",
    bind_methods      => [qw( Get_File )],
    bind_constructors => ['_open|do_open'],
);


