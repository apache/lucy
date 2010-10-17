package KinoSearch::Object::Num;
use KinoSearch;

1;

__END__

__BINDING__

my $float32_xs_code = <<'END_XS_CODE';
MODULE = KinoSearch   PACKAGE = KinoSearch::Object::Float32

SV*
new(either_sv, value)
    SV    *either_sv;
    float  value;
CODE:
{
    kino_Float32 *self = (kino_Float32*)XSBind_new_blank_obj(either_sv);
    kino_Float32_init(self, value);
    RETVAL = KINO_OBJ_TO_SV_NOINC(self);
}
OUTPUT: RETVAL
END_XS_CODE

my $float64_xs_code = <<'END_XS_CODE';
MODULE = KinoSearch   PACKAGE = KinoSearch::Object::Float64

SV*
new(either_sv, value)
    SV     *either_sv;
    double  value;
CODE:
{
    kino_Float64 *self = (kino_Float64*)XSBind_new_blank_obj(either_sv);
    kino_Float64_init(self, value);
    RETVAL = KINO_OBJ_TO_SV_NOINC(self);
}
OUTPUT: RETVAL
END_XS_CODE

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Object::Float32",
    xs_code      => $float32_xs_code,
    bind_methods => [qw( Set_Value Get_Value )],
);
Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Object::Float64",
    xs_code      => $float64_xs_code,
    bind_methods => [qw( Set_Value Get_Value )],
);


