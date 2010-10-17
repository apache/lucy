package KinoSearch::Search::MatchAllQuery;
use KinoSearch;

1;

__END__

__BINDING__

my $constructor = <<'END_CONSTRUCTOR';
    my $match_all_query = KinoSearch::Search::MatchAllQuery->new;
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Search::MatchAllQuery",
    bind_constructors => ["new"],
    make_pod          => { constructor => { sample => $constructor }, }
);


