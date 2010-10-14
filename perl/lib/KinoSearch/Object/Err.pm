package KinoSearch::Object::Err;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    use Scalar::Util qw( blessed );
    my $bg_merger;
    while (1) {
        $bg_merger = eval {
            KinoSearch::Index::BackgroundMerger->new( index => $index );
        };
        last if $bg_merger;
        if ( blessed($@) and $@->isa("KinoSearch::Store::LockErr") ) {
            warn "Retrying...\n";
        }
        else {
            # Re-throw.
            die "Failed to open BackgroundMerger: $@";
        }
    }
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Object::Err",
    bind_methods      => [qw( Cat_Mess Get_Mess )],
    make_pod          => { synopsis => $synopsis },
    bind_constructors => ["_new"],
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

