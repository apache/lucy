package KinoSearch::Search::Collector;
use KinoSearch;

1;

__END__

__BINDING__

my $constructor = <<'END_CONSTRUCTOR';
    package MyCollector;
    use base qw( KinoSearch::Search::Collector );
    our %foo;
    sub new {
        my $self = shift->SUPER::new;
        my %args = @_;
        $foo{$$self} = $args{foo};
        return $self;
    }
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Search::Collector",
    bind_methods => [
        qw(
            Collect
            Set_Reader
            Set_Base
            Set_Matcher
            Need_Score
            )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => "    # Abstract base class.\n",
        constructor => { sample => $constructor },
        methods     => [qw( collect )],
    },
);
Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Search::Collector::OffsetCollector",
    bind_constructors => ["new"],
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

