package KinoSearch::Store::LockErr;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    while (1) {
        my $bg_merger = eval {
            KinoSearch::Index::BackgroundMerger->new( index => $index );
        };
        if ( blessed($@) and $@->isa("KinoSearch::Store::LockErr") ) {
            warn "Retrying...\n";
        }
        elsif (!$bg_merger) {
            # Re-throw.
            die "Failed to open BackgroundMerger: $@";
        }
        ...
    }
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel     => "KinoSearch",
    class_name => "KinoSearch::Store::LockErr",
    make_pod   => { synopsis => $synopsis }
);


