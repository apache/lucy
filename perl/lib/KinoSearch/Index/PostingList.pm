package KinoSearch::Index::PostingList;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $posting_list_reader 
        = $seg_reader->obtain("KinoSearch::Index::PostingListReader");
    my $posting_list = $posting_list_reader->posting_list( 
        field => 'content',
        term  => 'foo',
    );
    while ( my $doc_id = $posting_list->next ) {
        say "Matching doc id: $doc_id";
    }
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Index::PostingList",
    bind_methods => [
        qw(
            Seek
            Get_Posting
            Get_Doc_Freq
            Make_Matcher
            )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis => $synopsis,
        methods  => [
            qw(
                next
                advance
                get_doc_id
                get_doc_freq
                seek
                )
        ],
    },
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.


