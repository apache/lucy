# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
package Clownfish::Build::Binding;
use strict;
use warnings;

our $VERSION = '0.003000';
$VERSION = eval $VERSION;

sub bind_all {
    my $class = shift;
    $class->bind_bytebuf;
    $class->bind_charbuf;
    $class->bind_err;
    $class->bind_hash;
    $class->bind_host;
    $class->bind_lockfreeregistry;
    $class->bind_float32;
    $class->bind_float64;
    $class->bind_obj;
    $class->bind_varray;
    $class->bind_vtable;
}

sub bind_bytebuf {
    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy     PACKAGE = Clownfish::ByteBuf

SV*
new(either_sv, sv)
    SV *either_sv;
    SV *sv;
CODE:
{
    STRLEN size;
    char *ptr = SvPV(sv, size);
    lucy_ByteBuf *self = (lucy_ByteBuf*)XSBind_new_blank_obj(either_sv);
    lucy_BB_init(self, size);
    Lucy_BB_Mimic_Bytes(self, ptr, size);
    RETVAL = CFISH_OBJ_TO_SV_NOINC(self);
}
OUTPUT: RETVAL

SV*
_deserialize(self, instream)
    lucy_ByteBuf *self;
    lucy_InStream *instream;
CODE:
    lucy_ByteBuf *thawed = Lucy_BB_Deserialize(self, instream);
    RETVAL = (SV*)Lucy_BB_To_Host(thawed);
OUTPUT: RETVAL
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Clownfish::ByteBuf",
    );
    $binding->append_xs($xs_code);
    $binding->exclude_constructor;

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_charbuf {
    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy     PACKAGE = Clownfish::CharBuf

SV*
new(either_sv, sv)
    SV *either_sv;
    SV *sv;
CODE:
{
    STRLEN size;
    char *ptr = SvPVutf8(sv, size);
    lucy_CharBuf *self = (lucy_CharBuf*)XSBind_new_blank_obj(either_sv);
    lucy_CB_init(self, size);
    Lucy_CB_Cat_Trusted_Str(self, ptr, size);
    RETVAL = CFISH_OBJ_TO_SV_NOINC(self);
}
OUTPUT: RETVAL

SV*
_clone(self)
    lucy_CharBuf *self;
CODE:
    RETVAL = CFISH_OBJ_TO_SV_NOINC(lucy_CB_clone(self));
OUTPUT: RETVAL

SV*
_deserialize(self, instream)
    lucy_CharBuf *self;
    lucy_InStream *instream;
CODE:
    lucy_CharBuf *thawed = Lucy_CB_Deserialize(self, instream);
    RETVAL = (SV*)Lucy_CB_To_Host(thawed);
OUTPUT: RETVAL

SV*
to_perl(self)
    lucy_CharBuf *self;
CODE:
    RETVAL = XSBind_cb_to_sv(self);
OUTPUT: RETVAL

MODULE = Lucy     PACKAGE = Clownfish::ViewCharBuf

SV*
_new(unused, sv)
    SV *unused;
    SV *sv;
CODE:
{
    STRLEN size;
    char *ptr = SvPVutf8(sv, size);
    lucy_ViewCharBuf *self
        = lucy_ViewCB_new_from_trusted_utf8(ptr, size);
    CHY_UNUSED_VAR(unused);
    RETVAL = CFISH_OBJ_TO_SV_NOINC(self);
}
OUTPUT: RETVAL
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Clownfish::CharBuf",
    );
    $binding->append_xs($xs_code);
    $binding->exclude_constructor;

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_err {
    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    package MyErr;
    use base qw( Clownfish::Err );
    
    ...
    
    package main;
    use Scalar::Util qw( blessed );
    while (1) {
        eval {
            do_stuff() or MyErr->throw("retry");
        };
        if ( blessed($@) and $@->isa("MyErr") ) {
            warn "Retrying...\n";
        }
        else {
            # Re-throw.
            die "do_stuff() died: $@";
        }
    }
END_SYNOPSIS
    $pod_spec->set_synopsis($synopsis);

    my $xs_code = <<'END_XS_CODE';
MODULE =  Lucy    PACKAGE = Clownfish::Err

SV*
trap(routine_sv, context_sv)
    SV *routine_sv;
    SV *context_sv;
CODE:
    cfish_Err *error = XSBind_trap(routine_sv, context_sv);
    RETVAL = CFISH_OBJ_TO_SV_NOINC(error);
OUTPUT: RETVAL
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Clownfish::Err",
    );
    $binding->bind_constructor( alias => '_new' );
    $binding->set_pod_spec($pod_spec);
    $binding->append_xs($xs_code);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_hash {
    my @hand_rolled = qw(
        Store
        Next
    );

    my $xs_code = <<'END_XS_CODE';
MODULE =  Lucy    PACKAGE = Clownfish::Hash

SV*
_deserialize(self, instream)
    lucy_Hash *self;
    lucy_InStream *instream;
CODE:
    lucy_Hash *thawed = Lucy_Hash_Deserialize(self, instream);
    RETVAL = (SV*)Lucy_Hash_To_Host(thawed);
OUTPUT: RETVAL

SV*
_fetch(self, key)
    lucy_Hash *self;
    const lucy_CharBuf *key;
CODE:
    RETVAL = CFISH_OBJ_TO_SV(lucy_Hash_fetch(self, (lucy_Obj*)key));
OUTPUT: RETVAL

void
store(self, key, value);
    lucy_Hash          *self;
    const lucy_CharBuf *key;
    lucy_Obj           *value;
PPCODE:
{
    if (value) { CFISH_INCREF(value); }
    lucy_Hash_store(self, (lucy_Obj*)key, value);
}

void
next(self)
    lucy_Hash *self;
PPCODE:
{
    lucy_Obj *key;
    lucy_Obj *val;

    if (Lucy_Hash_Next(self, &key, &val)) {
        SV *key_sv = (SV*)Lucy_Obj_To_Host(key);
        SV *val_sv = (SV*)Lucy_Obj_To_Host(val);

        XPUSHs(sv_2mortal(key_sv));
        XPUSHs(sv_2mortal(val_sv));
        XSRETURN(2);
    }
    else {
        XSRETURN_EMPTY;
    }
}
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Clownfish::Hash",
    );
    $binding->exclude_method($_) for @hand_rolled;
    $binding->append_xs($xs_code);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_host {
    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy     PACKAGE = Clownfish::Host

=for comment

These are all for testing purposes only.

=cut

IV
_test(...)
CODE:
    RETVAL = items;
OUTPUT: RETVAL

SV*
_test_obj(...)
CODE:
{
    lucy_ByteBuf *test_obj = lucy_BB_new_bytes("blah", 4);
    SV *pack_var = get_sv("Clownfish::Host::testobj", 1);
    RETVAL = (SV*)Lucy_BB_To_Host(test_obj);
    SvSetSV_nosteal(pack_var, RETVAL);
    CFISH_DECREF(test_obj);
    CHY_UNUSED_VAR(items);
}
OUTPUT: RETVAL

void
_callback(obj)
    lucy_Obj *obj;
PPCODE:
{
    lucy_ZombieCharBuf *blank = CFISH_ZCB_BLANK();
    lucy_Host_callback(obj, "_test", 2,
                       CFISH_ARG_OBJ("nothing", (lucy_CharBuf*)blank),
                       CFISH_ARG_I32("foo", 3));
}

int64_t
_callback_i64(obj)
    lucy_Obj *obj;
CODE:
{
    lucy_ZombieCharBuf *blank = CFISH_ZCB_BLANK();
    RETVAL
        = lucy_Host_callback_i64(obj, "_test", 2,
                                 CFISH_ARG_OBJ("nothing", (lucy_CharBuf*)blank),
                                 CFISH_ARG_I32("foo", 3));
}
OUTPUT: RETVAL

double
_callback_f64(obj)
    lucy_Obj *obj;
CODE:
{
    lucy_ZombieCharBuf *blank = CFISH_ZCB_BLANK();
    RETVAL
        = lucy_Host_callback_f64(obj, "_test", 2,
                                 CFISH_ARG_OBJ("nothing", (lucy_CharBuf*)blank),
                                 CFISH_ARG_I32("foo", 3));
}
OUTPUT: RETVAL

SV*
_callback_obj(obj)
    lucy_Obj *obj;
CODE:
{
    lucy_Obj *other = lucy_Host_callback_obj(obj, "_test_obj", 0);
    RETVAL = (SV*)Lucy_Obj_To_Host(other);
    CFISH_DECREF(other);
}
OUTPUT: RETVAL
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Clownfish::Host",
    );
    $binding->append_xs($xs_code);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_lockfreeregistry {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Clownfish::LockFreeRegistry",
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_float32 {
    my $float32_xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Clownfish::Float32

SV*
new(either_sv, value)
    SV    *either_sv;
    float  value;
CODE:
{
    lucy_Float32 *self = (lucy_Float32*)XSBind_new_blank_obj(either_sv);
    lucy_Float32_init(self, value);
    RETVAL = CFISH_OBJ_TO_SV_NOINC(self);
}
OUTPUT: RETVAL
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Clownfish::Float32",
    );
    $binding->append_xs($float32_xs_code);
    $binding->exclude_constructor;

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_float64 {
    my $float64_xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Clownfish::Float64

SV*
new(either_sv, value)
    SV     *either_sv;
    double  value;
CODE:
{
    lucy_Float64 *self = (lucy_Float64*)XSBind_new_blank_obj(either_sv);
    lucy_Float64_init(self, value);
    RETVAL = CFISH_OBJ_TO_SV_NOINC(self);
}
OUTPUT: RETVAL
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Clownfish::Float64",
    );
    $binding->append_xs($float64_xs_code);
    $binding->exclude_constructor;

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_obj {
    my @exposed = qw(
        To_String
        To_I64
        To_F64
        Equals
        Dump
        Load
    );
    my @hand_rolled = qw(
        Is_A
    );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    package MyObj;
    use base qw( Clownfish::Obj );
    
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
Clownfish::Obj is the base class of the Clownfish object hierarchy.

From the standpoint of a Perl programmer, all classes are implemented as
blessed scalar references, with the scalar storing a pointer to a C struct.

=head2 Subclassing

The recommended way to subclass Clownfish::Obj and its descendants is
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

=over

=item *

Inside-out aficionados will have noted that the "cached scalar id" stratagem
recommended above isn't compatible with ithreads.

=item *

Overridden methods must not return undef unless the API specifies that
returning undef is permissible.  (Failure to adhere to this rule currently
results in a segfault rather than an exception.)

=back

=head1 CONSTRUCTOR

=head2 new()

Abstract constructor -- must be invoked via a subclass.  Attempting to
instantiate objects of class "Clownfish::Obj" directly causes an
error.

Takes no arguments; if any are supplied, an error will be reported.

=head1 DESTRUCTOR

=head2 DESTROY

All Clownfish classes implement a DESTROY method; if you override it in a
subclass, you must call C<< $self->SUPER::DESTROY >> to avoid leaking memory.
END_DESCRIPTION
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->set_description($description);
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy     PACKAGE = Clownfish::Obj

chy_bool_t
is_a(self, class_name)
    lucy_Obj *self;
    const lucy_CharBuf *class_name;
CODE:
{
    lucy_VTable *target = lucy_VTable_fetch_vtable(class_name);
    RETVAL = Lucy_Obj_Is_A(self, target);
}
OUTPUT: RETVAL

void
STORABLE_freeze(self, ...)
    lucy_Obj *self;
PPCODE:
{
    CHY_UNUSED_VAR(self);
    if (items < 2 || !SvTRUE(ST(1))) {
        SV *retval;
        lucy_ByteBuf *serialized_bb;
        lucy_RAMFileHandle *file_handle
            = lucy_RAMFH_open(NULL, LUCY_FH_WRITE_ONLY | LUCY_FH_CREATE, NULL);
        lucy_OutStream *target = lucy_OutStream_open((lucy_Obj*)file_handle);

        Lucy_Obj_Serialize(self, target);

        Lucy_OutStream_Close(target);
        serialized_bb
            = Lucy_RAMFile_Get_Contents(Lucy_RAMFH_Get_File(file_handle));
        retval = XSBind_bb_to_sv(serialized_bb);
        CFISH_DECREF(file_handle);
        CFISH_DECREF(target);

        if (SvCUR(retval) == 0) { // Thwart Storable bug
            THROW(LUCY_ERR, "Calling serialize produced an empty string");
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
    lucy_ZombieCharBuf *klass
        = CFISH_ZCB_WRAP_STR(class_name, strlen(class_name));
    lucy_VTable *vtable
        = (lucy_VTable*)lucy_VTable_singleton((lucy_CharBuf*)klass, NULL);
    STRLEN len;
    char *ptr = SvPV(serialized_sv, len);
    lucy_ViewByteBuf *contents = lucy_ViewBB_new(ptr, len);
    lucy_RAMFile *ram_file = lucy_RAMFile_new((lucy_ByteBuf*)contents, true);
    lucy_RAMFileHandle *file_handle
        = lucy_RAMFH_open(NULL, LUCY_FH_READ_ONLY, ram_file);
    lucy_InStream *instream = lucy_InStream_open((lucy_Obj*)file_handle);
    lucy_Obj *self = Lucy_VTable_Foster_Obj(vtable, blank_obj);
    lucy_Obj *deserialized = Lucy_Obj_Deserialize(self, instream);

    CHY_UNUSED_VAR(cloning);
    CFISH_DECREF(contents);
    CFISH_DECREF(ram_file);
    CFISH_DECREF(file_handle);
    CFISH_DECREF(instream);

    // Catch bad deserialize() override.
    if (deserialized != self) {
        THROW(LUCY_ERR, "Error when deserializing obj of class %o", klass);
    }
}
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Clownfish::Obj",
    );
    $binding->bind_method( alias => '_load', method => 'Load' );
    $binding->exclude_method($_) for @hand_rolled;
    $binding->append_xs($xs_code);
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_varray {
    my @hand_rolled = qw(
        Shallow_Copy
        Shift
        Pop
        Delete
        Store
        Fetch
    );

    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Clownfish::VArray

SV*
shallow_copy(self)
    lucy_VArray *self;
CODE:
    RETVAL = CFISH_OBJ_TO_SV_NOINC(Lucy_VA_Shallow_Copy(self));
OUTPUT: RETVAL

SV*
_deserialize(self, instream)
    lucy_VArray *self;
    lucy_InStream *instream;
CODE:
    lucy_VArray *thawed = Lucy_VA_Deserialize(self, instream);
    RETVAL = (SV*)Lucy_VA_To_Host(thawed);
OUTPUT: RETVAL

SV*
_clone(self)
    lucy_VArray *self;
CODE:
    RETVAL = CFISH_OBJ_TO_SV_NOINC(Lucy_VA_Clone(self));
OUTPUT: RETVAL

SV*
shift(self)
    lucy_VArray *self;
CODE:
    RETVAL = CFISH_OBJ_TO_SV_NOINC(Lucy_VA_Shift(self));
OUTPUT: RETVAL

SV*
pop(self)
    lucy_VArray *self;
CODE:
    RETVAL = CFISH_OBJ_TO_SV_NOINC(Lucy_VA_Pop(self));
OUTPUT: RETVAL

SV*
delete(self, tick)
    lucy_VArray *self;
    uint32_t    tick;
CODE:
    RETVAL = CFISH_OBJ_TO_SV_NOINC(Lucy_VA_Delete(self, tick));
OUTPUT: RETVAL

void
store(self, tick, value);
    lucy_VArray *self;
    uint32_t     tick;
    lucy_Obj    *value;
PPCODE:
{
    if (value) { CFISH_INCREF(value); }
    lucy_VA_store(self, tick, value);
}

SV*
fetch(self, tick)
    lucy_VArray *self;
    uint32_t     tick;
CODE:
    RETVAL = CFISH_OBJ_TO_SV(Lucy_VA_Fetch(self, tick));
OUTPUT: RETVAL
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Clownfish::VArray",
    );
    $binding->exclude_method($_) for @hand_rolled;
    $binding->append_xs($xs_code);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_vtable {
    my @hand_rolled = qw( Make_Obj );

    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Clownfish::VTable

SV*
_get_registry()
CODE:
    if (lucy_VTable_registry == NULL) {
        lucy_VTable_init_registry();
    }
    RETVAL = (SV*)Lucy_Obj_To_Host((lucy_Obj*)lucy_VTable_registry);
OUTPUT: RETVAL

SV*
singleton(unused_sv, ...)
    SV *unused_sv;
CODE:
{
    CHY_UNUSED_VAR(unused_sv);
    lucy_CharBuf *class_name = NULL;
    lucy_VTable  *parent     = NULL;
    chy_bool_t args_ok
        = XSBind_allot_params(&(ST(0)), 1, items,
                              ALLOT_OBJ(&class_name, "class_name", 10, true,
                                        LUCY_CHARBUF, alloca(cfish_ZCB_size())),
                              ALLOT_OBJ(&parent, "parent", 6, false,
                                        LUCY_VTABLE, NULL),
                              NULL);
    if (!args_ok) {
        CFISH_RETHROW(CFISH_INCREF(cfish_Err_get_error()));
    }
    lucy_VTable *singleton = lucy_VTable_singleton(class_name, parent);
    RETVAL = (SV*)Lucy_VTable_To_Host(singleton);
}
OUTPUT: RETVAL

SV*
make_obj(self)
    lucy_VTable *self;
CODE:
    lucy_Obj *blank = Lucy_VTable_Make_Obj(self);
    RETVAL = CFISH_OBJ_TO_SV_NOINC(blank);
OUTPUT: RETVAL
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Clownfish::VTable",
    );
    $binding->exclude_method($_) for @hand_rolled;
    $binding->append_xs($xs_code);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

1;
