package KinoSearch::Search::Collector::SortCollector;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Search::Collector::SortCollector",
    bind_methods      => [qw( Pop_Match_Docs Get_Total_Hits )],
    bind_constructors => ["new"],
);


