package KinoSearch::Object::Obj;
use KinoSearch;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = KinoSearch     PACKAGE = KinoSearch::Object::Obj

chy_bool_t
is_a(self, class_name)
    kino_Obj *self;
    const kino_CharBuf *class_name;
CODE:
{
    kino_VTable *target = kino_VTable_fetch_vtable(class_name);
    RETVAL = Kino_Obj_Is_A(self, target);
}
OUTPUT: RETVAL

void
STORABLE_freeze(self, ...)
    kino_Obj *self;
PPCODE:
{
    CHY_UNUSED_VAR(self);
    if (items < 2 || !SvTRUE(ST(1))) {
        SV *retval;
        kino_ByteBuf *serialized_bb;
        kino_RAMFileHandle *file_handle = kino_RAMFH_open(NULL, 
            KINO_FH_WRITE_ONLY | KINO_FH_CREATE, NULL);
        kino_OutStream *target = kino_OutStream_open((kino_Obj*)file_handle);

        Kino_Obj_Serialize(self, target);

        Kino_OutStream_Close(target);
        serialized_bb = Kino_RAMFile_Get_Contents(
            Kino_RAMFH_Get_File(file_handle));
        retval = XSBind_bb_to_sv(serialized_bb);
        KINO_DECREF(file_handle);
        KINO_DECREF(target);

        if (SvCUR(retval) == 0) { // Thwart Storable bug 
            THROW(KINO_ERR, "Calling serialize produced an empty string");
        }
        ST(0) = sv_2mortal(retval);
        XSRETURN(1);
    }
}

=begin comment

Calls deserialize(), and copies the object pointer.  Since deserialize is an
abstract method, it will confess() unless implemented.

=end comment
=cut

void
STORABLE_thaw(blank_obj, cloning, serialized_sv)
    SV *blank_obj;
    SV *cloning;
    SV *serialized_sv;
PPCODE:
{
    char *class_name = HvNAME(SvSTASH(SvRV(blank_obj)));
    kino_ZombieCharBuf *klass 
        = KINO_ZCB_WRAP_STR(class_name, strlen(class_name));
    kino_VTable *vtable = (kino_VTable*)kino_VTable_singleton(
        (kino_CharBuf*)klass, NULL);
    STRLEN len;
    char *ptr = SvPV(serialized_sv, len);
    kino_ViewByteBuf *contents = kino_ViewBB_new(ptr, len);
    kino_RAMFile *ram_file = kino_RAMFile_new((kino_ByteBuf*)contents, true);
    kino_RAMFileHandle *file_handle 
        = kino_RAMFH_open(NULL, KINO_FH_READ_ONLY, ram_file);
    kino_InStream *instream = kino_InStream_open((kino_Obj*)file_handle);
    kino_Obj *self = Kino_VTable_Foster_Obj(vtable, blank_obj);
    kino_Obj *deserialized = Kino_Obj_Deserialize(self, instream);

    CHY_UNUSED_VAR(cloning);
    KINO_DECREF(contents);
    KINO_DECREF(ram_file);
    KINO_DECREF(file_handle);
    KINO_DECREF(instream);

    // Catch bad deserialize() override. 
    if (deserialized != self) {
        THROW(KINO_ERR, "Error when deserializing obj of class %o", klass);
    }
}

void
DESTROY(self)
    kino_Obj *self;
PPCODE:
    /*
    {
        char *perl_class = HvNAME(SvSTASH(SvRV(ST(0))));
        warn("Destroying: 0x%x %s", (unsigned)self, perl_class);
    }
    */
    Kino_Obj_Destroy(self);
END_XS_CODE

my $synopsis = <<'END_SYNOPSIS';
    package MyObj;
    use base qw( KinoSearch::Object::Obj );
    
    # Inside-out member var.
    my %foo;
    
    sub new {
        my ( $class, %args ) = @_;
        my $foo = delete $args{foo};
        my $self = $class->SUPER::new(%args);
        $foo{$$self} = $foo;
        return $self;
    }
    
    sub get_foo {
        my $self = shift;
        return $foo{$$self};
    }
    
    sub DESTROY {
        my $self = shift;
        delete $foo{$$self};
        $self->SUPER::DESTROY;
    }
END_SYNOPSIS

my $description = <<'END_DESCRIPTION';
All objects in the KinoSearch:: hierarchy descend from
KinoSearch::Object::Obj.  All classes are implemented as blessed scalar
references, with the scalar storing a pointer to a C struct.

==head2 Subclassing

The recommended way to subclass KinoSearch::Object::Obj and its descendants is
to use the inside-out design pattern.  (See L<Class::InsideOut> for an
introduction to inside-out techniques.)

Since the blessed scalar stores a C pointer value which is unique per-object,
C<$$self> can be used as an inside-out ID.

    # Accessor for 'foo' member variable.
    sub get_foo {
        my $self = shift;
        return $foo{$$self};
    }


Caveats:

==over

==item *

Inside-out aficionados will have noted that the "cached scalar id" stratagem
recommended above isn't compatible with ithreads -- but KinoSearch doesn't
support ithreads anyway, so it doesn't matter.

==item *

Overridden methods must not return undef unless the API specifies that
returning undef is permissible.  (Failure to adhere to this rule currently
results in a segfault rather than an exception.)

==back

==head1 CONSTRUCTOR

==head2 new()

Abstract constructor -- must be invoked via a subclass.  Attempting to
instantiate objects of class "KinoSearch::Object::Obj" directly causes an
error.

Takes no arguments; if any are supplied, an error will be reported.

==head1 DESTRUCTOR

==head2 DESTROY

All KinoSearch classes implement a DESTROY method; if you override it in a
subclass, you must call C<< $self->SUPER::DESTROY >> to avoid leaking memory.
END_DESCRIPTION

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Object::Obj",
    xs_code      => $xs_code,
    bind_methods => [
        qw(
            Get_RefCount
            Inc_RefCount
            Dec_RefCount
            Get_VTable
            To_String
            To_I64
            To_F64
            Dump
            _load|Load
            Clone
            Mimic
            Equals
            Hash_Sum
            Serialize
            Deserialize
            Destroy
            )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        description => $description,
        methods     => [
            qw(
                to_string
                to_i64
                to_f64
                equals
                dump
                load
                )
        ],
    }
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

