use Lucy;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy     PACKAGE = Lucy::Object::Obj

chy_bool_t
is_a(self, class_name)
    lucy_Obj *self;
    lucy_ZombieCharBuf class_name;
CODE:
{
    lucy_VTable *target = lucy_VTable_fetch_vtable((lucy_CharBuf*)&class_name);
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
        lucy_RAMFileHandle *file_handle = lucy_RAMFH_open(NULL, 
            LUCY_FH_WRITE_ONLY | LUCY_FH_CREATE, NULL);
        lucy_OutStream *target = lucy_OutStream_open((lucy_Obj*)file_handle);

        Lucy_Obj_Serialize(self, target);

        Lucy_OutStream_Close(target);
        serialized_bb = Lucy_RAMFile_Get_Contents(
            Lucy_RAMFH_Get_File(file_handle));
        retval = XSBind_bb_to_sv(serialized_bb);
        LUCY_DECREF(file_handle);
        LUCY_DECREF(target);

        if (SvCUR(retval) == 0) { /* Thwart Storable bug */
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
    lucy_ZombieCharBuf klass 
        = lucy_ZCB_make_str(class_name, strlen(class_name));
    lucy_VTable *vtable = (lucy_VTable*)lucy_VTable_singleton(
        (lucy_CharBuf*)&klass, NULL);
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
    LUCY_DECREF(contents);
    LUCY_DECREF(ram_file);
    LUCY_DECREF(file_handle);
    LUCY_DECREF(instream);

    /* Catch bad deserialize() override. */
    if (deserialized != self) {
        THROW(LUCY_ERR, "Error when deserializing obj of class %o", &klass);
    }
}

void
DESTROY(self)
    lucy_Obj *self;
PPCODE:
    /*
    {
        char *perl_class = HvNAME(SvSTASH(SvRV(ST(0))));
        warn("Destroying: 0x%x %s", (unsigned)self, perl_class);
    }
    */
    Lucy_Obj_Destroy(self);
END_XS_CODE

Boilerplater::Binding::Perl::Class->register(
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
            Hash_Code
            Serialize
            Deserialize
            Destroy
            )
    ],
    bind_constructors => ["new"],
);

__COPYRIGHT__

    /**
     * Copyright 2009 The Apache Software Foundation
     *
     * Licensed under the Apache License, Version 2.0 (the "License");
     * you may not use this file except in compliance with the License.
     * You may obtain a copy of the License at
     *
     *     http://www.apache.org/licenses/LICENSE-2.0
     *
     * Unless required by applicable law or agreed to in writing, software
     * distributed under the License is distributed on an "AS IS" BASIS,
     * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
     * implied.  See the License for the specific language governing
     * permissions and limitations under the License.
     */

