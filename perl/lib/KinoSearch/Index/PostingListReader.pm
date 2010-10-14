package KinoSearch::Index::PostingListReader;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $posting_list_reader 
        = $seg_reader->obtain("KinoSearch::Index::PostingListReader");
    my $posting_list = $posting_list_reader->posting_list(
        field => 'title', 
        term  => 'foo',
    );
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::PostingListReader",
    bind_constructors => ["new"],
    bind_methods      => [qw( Posting_List Get_Lex_Reader )],
    make_pod          => {
        synopsis => $synopsis,
        methods  => [qw( posting_list )],
    },
);
Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::DefaultPostingListReader",
    bind_constructors => ["new"],
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.
