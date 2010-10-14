package KinoSearch::Search::Query;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    # Query is an abstract base class.
    package MyQuery;
    use base qw( KinoSearch::Search::Query );
    
    sub make_compiler {
        my $self = shift;
        return MyCompiler->new( @_, parent => $self );
    }
    
    package MyCompiler;
    use base ( KinoSearch::Search::Compiler );
    ...
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR_CODE_SAMPLE';
    my $query = MyQuery->SUPER::new(
        boost => 2.5,
    );
END_CONSTRUCTOR_CODE_SAMPLE

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Search::Query",
    bind_methods => [
        qw( Set_Boost
            Get_Boost
            _make_compiler|Make_Compiler )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
        methods     => [qw( make_compiler set_boost get_boost )],
    },
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

