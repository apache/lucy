package KinoSearch::Index::DocReader;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $doc_reader = $seg_reader->obtain("KinoSearch::Index::DocReader");
    my $doc        = $doc_reader->fetch( doc_id => $doc_id );
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::DocReader",
    bind_constructors => ["new"],
    bind_methods      => [qw( Fetch )],
    make_pod          => {
        synopsis => $synopsis,
        methods  => [qw( fetch aggregator )],
    },
);
Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::DefaultDocReader",
    bind_constructors => ["new"],
);


