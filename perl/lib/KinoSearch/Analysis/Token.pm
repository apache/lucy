package KinoSearch::Analysis::Token;
use KinoSearch;

1;

__END__

__BINDING__

my $xs = <<'END_XS';
MODULE = KinoSearch    PACKAGE = KinoSearch::Analysis::Token

SV*
new(either_sv, ...)
    SV *either_sv;
CODE:
{
    SV *text_sv         = NULL;
    SV *start_offset_sv = NULL;
    SV *end_offset_sv   = NULL;
    SV *pos_inc_sv      = NULL;
    SV *boost_sv        = NULL;

    XSBind_allot_params( &(ST(0)), 1, items, 
        "KinoSearch::Analysis::Token::new_PARAMS",
        &text_sv, "text", 4,
        &start_offset_sv, "start_offset", 12, 
        &end_offset_sv, "end_offset", 10, 
        &pos_inc_sv, "pos_inc", 7, 
        &boost_sv, "boost", 5, 
        NULL);

    if (!XSBind_sv_defined(text_sv)) { 
        THROW(KINO_ERR, "Missing required param 'text'"); 
    }
    if (!XSBind_sv_defined(start_offset_sv)) { 
        THROW(KINO_ERR, "Missing required param 'start_offset'"); 
    }
    if (!XSBind_sv_defined(end_offset_sv)) { 
        THROW(KINO_ERR, "Missing required param 'end_offset'"); 
    }

    STRLEN      len;
    char       *text      = SvPVutf8(text_sv, len);
    uint32_t    start_off = SvUV(start_offset_sv);
    uint32_t    end_off   = SvUV(end_offset_sv);
    int32_t     pos_inc   = pos_inc_sv ? SvIV(pos_inc_sv) : 1;
    float       boost     = boost_sv ? (float)SvNV(boost_sv) : 1.0f;
    kino_Token *self   = (kino_Token*)XSBind_new_blank_obj(either_sv);
    kino_Token_init(self, text, len, start_off, end_off, boost, 
        pos_inc);
    RETVAL = KINO_OBJ_TO_SV_NOINC(self);
}
OUTPUT: RETVAL

SV*
get_text(self)
    kino_Token *self;
CODE:
    RETVAL = newSVpvn(Kino_Token_Get_Text(self), Kino_Token_Get_Len(self));
    SvUTF8_on(RETVAL);
OUTPUT: RETVAL

void
set_text(self, sv)
    kino_Token *self;
    SV *sv;
PPCODE:
{
    STRLEN len;
    char *ptr = SvPVutf8(sv, len);
    Kino_Token_Set_Text(self, ptr, len);
}
END_XS

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Analysis::Token",
    bind_methods => [
        qw(
            Get_Start_Offset
            Get_End_Offset
            Get_Boost
            Get_Pos_Inc
            )
    ],
    xs_code => $xs,
);

__POD__

=head1 NAME

KinoSearch::Analysis::Token - Redacted.

=head1 REDACTED

Token's public API has been redacted.

=head1 COPYRIGHT

Copyright 2005-2010 Marvin Humphrey

=head1 LICENSE, DISCLAIMER, BUGS, etc.

See L<KinoSearch> version 0.30.

=cut

