package KinoSearch::Index::DeletionsReader;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::DeletionsReader",
    bind_constructors => ['new'],
    bind_methods      => [qw( Iterator Del_Count )],
);
Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::DefaultDeletionsReader",
    bind_constructors => ['new'],
    bind_methods      => [qw( Read_Deletions )],
);


