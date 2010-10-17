package KinoSearch::Analysis::Analyzer;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Analysis::Analyzer",
    bind_methods      => [qw( Transform Transform_Text Split )],
    bind_constructors => ["new"],
    make_pod          => { synopsis => "    # Abstract base class.\n", }
);


