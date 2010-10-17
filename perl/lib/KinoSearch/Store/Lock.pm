package KinoSearch::Store::Lock;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $lock = $lock_factory->make_lock(
        name    => 'write',
        timeout => 5000,
    );
    $lock->obtain or die "can't get lock for " . $lock->get_name;
    do_stuff();
    $lock->release;
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $lock = KinoSearch::Store::Lock->new(
        name     => 'commit',     # required
        folder   => $folder,      # required
        host     => $hostname,    # required
        timeout  => 5000,         # default: 0
        interval => 1000,         # default: 100
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Store::Lock",
    bind_methods => [
        qw(
            Obtain
            Request
            Is_Locked
            Release
            Clear_Stale
            Get_Name
            Get_Lock_Path
            Get_Host
            )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
        methods     => [
            qw(
                obtain
                request
                release
                is_locked
                clear_stale
                )
        ],
    },
);
Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Store::LockFileLock",
    bind_constructors => ["new"],
);
Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Store::SharedLock",
    bind_constructors => ["new"],
);


