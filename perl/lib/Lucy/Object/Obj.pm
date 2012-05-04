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

package Lucy::Object::Obj;
use Lucy;
our $VERSION = '0.003001';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy     PACKAGE = Lucy::Object::Obj

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

my $synopsis = <<'END_SYNOPSIS';
    package MyObj;
    use base qw( Lucy::Object::Obj );
    
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
All objects in the Lucy:: hierarchy descend from
Lucy::Object::Obj.  All classes are implemented as blessed scalar
references, with the scalar storing a pointer to a C struct.

==head2 Subclassing

The recommended way to subclass Lucy::Object::Obj and its descendants is
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
recommended above isn't compatible with ithreads -- but Lucy doesn't
support ithreads anyway, so it doesn't matter.

==item *

Overridden methods must not return undef unless the API specifies that
returning undef is permissible.  (Failure to adhere to this rule currently
results in a segfault rather than an exception.)

==back

==head1 CONSTRUCTOR

==head2 new()

Abstract constructor -- must be invoked via a subclass.  Attempting to
instantiate objects of class "Lucy::Object::Obj" directly causes an
error.

Takes no arguments; if any are supplied, an error will be reported.

==head1 DESTRUCTOR

==head2 DESTROY

All Lucy classes implement a DESTROY method; if you override it in a
subclass, you must call C<< $self->SUPER::DESTROY >> to avoid leaking memory.
END_DESCRIPTION

Clownfish::CFC::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Object::Obj",
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


