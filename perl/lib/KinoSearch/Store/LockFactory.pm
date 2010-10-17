package KinoSearch::Store::LockFactory;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    use Sys::Hostname qw( hostname );
    my $hostname = hostname() or die "Can't get unique hostname";
    my $folder = KinoSearch::Store::FSFolder->new( 
        path => '/path/to/index', 
    );
    my $lock_factory = KinoSearch::Store::LockFactory->new(
        folder => $folder,
        host   => $hostname,
    );
    my $write_lock = $lock_factory->make_lock(
        name     => 'write',
        timeout  => 5000,
        interval => 100,
    );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $lock_factory = KinoSearch::Store::LockFactory->new(
        folder => $folder,      # required
        host   => $hostname,    # required
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Store::LockFactory",
    bind_methods      => [qw( Make_Lock Make_Shared_Lock )],
    bind_constructors => ["new"],
    make_pod          => {
        methods     => [qw( make_lock make_shared_lock)],
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    }
);


