package KinoSearch::Index::Similarity;
use KinoSearch;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = KinoSearch    PACKAGE = KinoSearch::Index::Similarity     

SV*
get_norm_decoder(self)
    kino_Similarity *self;
CODE:
    RETVAL = newSVpvn( (char*)Kino_Sim_Get_Norm_Decoder(self), 
        (256 * sizeof(float)) );
OUTPUT: RETVAL
END_XS_CODE

my $synopsis = <<'END_SYNOPSIS';
    package MySimilarity;

    sub length_norm { return 1.0 }    # disable length normalization

    package MyFullTextType;
    use base qw( KinoSearch::Plan::FullTextType );

    sub make_similarity { MySimilarity->new }
END_SYNOPSIS

my $constructor = qq|    my \$sim = KinoSearch::Index::Similarity->new;\n|;

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Index::Similarity",
    xs_code      => $xs_code,
    bind_methods => [
        qw( IDF
            TF
            Encode_Norm
            Decode_Norm
            Query_Norm
            Length_Norm
            Coord )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
        methods     => [qw( length_norm )],
    }
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.


