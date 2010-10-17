package KinoSearch::Index::SortCache;
use KinoSearch;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = KinoSearch   PACKAGE = KinoSearch::Index::SortCache

SV*
value(self, ...)
    kino_SortCache *self;
CODE:
{
    SV *ord_sv = NULL;
    int32_t ord = 0;

    XSBind_allot_params( &(ST(0)), 1, items, 
        "KinoSearch::Index::SortCache::value_PARAMS",
        &ord_sv, "ord", 3, 
        NULL);
    if (ord_sv) { ord = SvIV(ord_sv); }
    else { THROW(KINO_ERR, "Missing required param 'ord'"); }

    {
        kino_Obj *blank = Kino_SortCache_Make_Blank(self);
        kino_Obj *value = Kino_SortCache_Value(self, ord, blank);
        RETVAL = XSBind_kino_to_perl(value);
        KINO_DECREF(blank);
    }
}
OUTPUT: RETVAL
END_XS_CODE

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::SortCache",
    xs_code           => $xs_code,
    bind_constructors => ["new"],
    bind_methods      => [qw( Ordinal Find )],
);


