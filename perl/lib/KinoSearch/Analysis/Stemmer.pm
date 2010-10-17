package KinoSearch::Analysis::Stemmer;
use KinoSearch;

1;

__END__

__BINDING__

my $xs = <<'END_XS';
MODULE = KinoSearch    PACKAGE = KinoSearch::Analysis::Stemmer

void
_copy_snowball_symbols()
PPCODE:
{
    SV **const new_sv_ptr = hv_fetch(PL_modglobal,
        "Lingua::Stem::Snowball::sb_stemmer_new", 38, 0);
    SV **const delete_sv_ptr = hv_fetch(PL_modglobal,
        "Lingua::Stem::Snowball::sb_stemmer_delete", 41, 0);
    SV **const stem_sv_ptr = hv_fetch(PL_modglobal,
        "Lingua::Stem::Snowball::sb_stemmer_stem", 39, 0);
    SV **const length_sv_ptr = hv_fetch(PL_modglobal,
        "Lingua::Stem::Snowball::sb_stemmer_length", 41, 0);
    if (!new_sv_ptr || !delete_sv_ptr || !stem_sv_ptr || !length_sv_ptr) {
        THROW(KINO_ERR, "Failed to retrieve one or more Snowball symbols");
    }
    kino_Stemmer_sb_stemmer_new 
        = (kino_Stemmer_sb_stemmer_new_t)SvIV(*new_sv_ptr);
    kino_Stemmer_sb_stemmer_delete 
        = (kino_Stemmer_sb_stemmer_delete_t)SvIV(*delete_sv_ptr);
    kino_Stemmer_sb_stemmer_stem 
        = (kino_Stemmer_sb_stemmer_stem_t)SvIV(*stem_sv_ptr);
    kino_Stemmer_sb_stemmer_length 
        = (kino_Stemmer_sb_stemmer_length_t)SvIV(*length_sv_ptr);
}
END_XS

my $synopsis = <<'END_SYNOPSIS';
    my $stemmer = KinoSearch::Analysis::Stemmer->new( language => 'es' );
    
    my $polyanalyzer = KinoSearch::Analysis::PolyAnalyzer->new(
        analyzers => [ $case_folder, $tokenizer, $stemmer ],
    );

This class is a wrapper around L<Lingua::Stem::Snowball>, so it supports the
same languages.  
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $stemmer = KinoSearch::Analysis::Stemmer->new( language => 'es' );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Analysis::Stemmer",
    bind_constructors => ["new"],
    xs_code           => $xs,
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor }
    },
);


