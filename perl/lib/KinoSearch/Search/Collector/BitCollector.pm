package KinoSearch::Search::Collector::BitCollector;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $bit_vec = KinoSearch::Object::BitVector->new(
        capacity => $searcher->doc_max + 1,
    );
    my $bit_collector = KinoSearch::Search::Collector::BitCollector->new(
        bit_vector => $bit_vec, 
    );
    $searcher->collect(
        collector => $bit_collector,
        query     => $query,
    );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $bit_collector = KinoSearch::Search::Collector::BitCollector->new(
        bit_vector => $bit_vec,    # required
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Search::Collector::BitCollector",
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
        methods     => [qw( collect )],
    },
);


