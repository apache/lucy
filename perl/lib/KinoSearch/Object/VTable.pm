package KinoSearch::Object::VTable;
use KinoSearch;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = KinoSearch   PACKAGE = KinoSearch::Object::VTable

SV*
_get_registry()
CODE:
    if (kino_VTable_registry == NULL)
        kino_VTable_init_registry();
    RETVAL = (SV*)Kino_Obj_To_Host((kino_Obj*)kino_VTable_registry);
OUTPUT: RETVAL
END_XS_CODE

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Object::VTable",
    xs_code      => $xs_code,
    bind_methods => [qw( Get_Name Get_Parent )],
);


