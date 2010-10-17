package KinoSearch::Analysis::Inversion;
use KinoSearch;

1;

__END__

__BINDING__

my $xs = <<'END_XS';
MODULE = KinoSearch   PACKAGE = KinoSearch::Analysis::Inversion

SV*
new(...)
CODE:
{
    kino_Token *starter_token = NULL;
    // parse params, only if there's more than one arg 
    if (items > 1) {
        SV *text_sv = NULL;
        XSBind_allot_params( &(ST(0)), 1, items, 
            "KinoSearch::Analysis::Inversion::new_PARAMS",
            &text_sv, "text", 4,
            NULL);
        if (XSBind_sv_defined(text_sv)) {
            STRLEN len;
            char *text = SvPVutf8(text_sv, len);
            starter_token = kino_Token_new(text, len, 0, len, 1.0, 1);
        }
    }
        
    RETVAL = KINO_OBJ_TO_SV_NOINC(kino_Inversion_new(starter_token));
    KINO_DECREF(starter_token);
}
OUTPUT: RETVAL
END_XS

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Analysis::Inversion",
    bind_methods => [qw( Append Reset Invert Next )],
    xs_code      => $xs,
);


