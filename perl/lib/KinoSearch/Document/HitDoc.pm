package KinoSearch::Document::HitDoc;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    while ( my $hit_doc = $hits->next ) {
        print "$hit_doc->{title}\n";
        print $hit_doc->get_score . "\n";
        ...
    }
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Document::HitDoc",
    bind_constructors => ['new'],
    bind_methods      => [qw( Set_Score Get_Score )],
    make_pod          => {
        methods  => [qw( set_score get_score )],
        synopsis => $synopsis,
    },
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.
