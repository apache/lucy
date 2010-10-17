package KinoSearch::Search::NoMatchQuery;
use KinoSearch;

1;

__END__

__BINDING__

my $constructor = <<'END_CONSTRUCTOR';
    my $no_match_query = KinoSearch::Search::NoMatchQuery->new;
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Search::NoMatchQuery",
    bind_constructors => ["new"],
    make_pod          => { constructor => { sample => $constructor }, }
);


