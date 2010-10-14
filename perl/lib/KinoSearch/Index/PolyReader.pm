package KinoSearch::Index::PolyReader;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $polyreader = KinoSearch::Index::IndexReader->open( 
        index => '/path/to/index',
    );
    my $doc_reader = $polyreader->obtain("KinoSearch::Index::DocReader");
    for my $doc_id ( 1 .. $polyreader->doc_max ) {
        my $doc = $doc_reader->fetch($doc_id);
        print " $doc_id: $doc->{title}\n";
    }
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::PolyReader",
    bind_constructors => [ 'new', 'open|do_open' ],
    bind_methods      => [qw( Get_Seg_Readers )],
    make_pod          => { synopsis => $synopsis },
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

