use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    use Sys::Hostname qw( hostname );
    my $hostname = hostname() or die "Can't get unique hostname";
    my $manager = KinoSearch::Index::IndexManager->new( host => $hostname );

    # Index time:
    my $indexer = KinoSearch::Index::Indexer->new(
        index   => '/path/to/index',
        manager => $manager,
    );

    # Search time:
    my $reader = KinoSearch::Index::IndexReader->open(
        index   => '/path/to/index',
        manager => $manager,
    );
    my $searcher = KinoSearch::Search::IndexSearcher->new( index => $reader );
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel     => "KinoSearch",
    class_name => "KinoSearch::Docs::FileLocking",
    make_pod   => { synopsis => $synopsis, },
);


