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
package Lucy::Build::Binding::Misc;
use strict;
use warnings;

our $VERSION = '0.003000';
$VERSION = eval $VERSION;

sub bind_all {
    my $class = shift;
    $class->bind_lucy;
    $class->bind_cfish_test;
    $class->bind_lucy_test;
    $class->bind_testschema;
    $class->bind_bbsortex;
}

sub bind_lucy {
    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy    PACKAGE = Lucy

BOOT:
    cfish_Lucy_bootstrap();

IV
_dummy_function()
CODE:
    RETVAL = 1;
OUTPUT:
    RETVAL

SV*
to_clownfish(sv)
    SV *sv;
CODE:
{
    cfish_Obj *obj = XSBind_perl_to_cfish(sv);
    RETVAL = CFISH_OBJ_TO_SV_NOINC(obj);
}
OUTPUT: RETVAL

SV*
to_perl(sv)
    SV *sv;
CODE:
{
    if (sv_isobject(sv) && sv_derived_from(sv, "Clownfish::Obj")) {
        IV tmp = SvIV(SvRV(sv));
        cfish_Obj* obj = INT2PTR(cfish_Obj*, tmp);
        RETVAL = XSBind_cfish_to_perl(obj);
    }
    else {
        RETVAL = newSVsv(sv);
    }
}
OUTPUT: RETVAL

void
STORABLE_freeze(self, ...)
    cfish_Obj *self;
PPCODE:
{
    CHY_UNUSED_VAR(self);
    if (items < 2 || !SvTRUE(ST(1))) {
        SV *retval;
        cfish_ByteBuf *serialized_bb;
        lucy_RAMFileHandle *file_handle
            = lucy_RAMFH_open(NULL, LUCY_FH_WRITE_ONLY | LUCY_FH_CREATE, NULL);
        lucy_OutStream *target = lucy_OutStream_open((cfish_Obj*)file_handle);

        lucy_Freezer_serialize(self, target);

        Lucy_OutStream_Close(target);
        serialized_bb
            = Lucy_RAMFile_Get_Contents(Lucy_RAMFH_Get_File(file_handle));
        retval = XSBind_bb_to_sv(serialized_bb);
        CFISH_DECREF(file_handle);
        CFISH_DECREF(target);

        if (SvCUR(retval) == 0) { // Thwart Storable bug
            THROW(CFISH_ERR, "Calling serialize produced an empty string");
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
    cfish_ZombieCharBuf *klass
        = CFISH_ZCB_WRAP_STR(class_name, strlen(class_name));
    cfish_VTable *vtable
        = (cfish_VTable*)cfish_VTable_singleton((cfish_CharBuf*)klass, NULL);
    STRLEN len;
    char *ptr = SvPV(serialized_sv, len);
    cfish_ViewByteBuf *contents = cfish_ViewBB_new(ptr, len);
    lucy_RAMFile *ram_file = lucy_RAMFile_new((cfish_ByteBuf*)contents, true);
    lucy_RAMFileHandle *file_handle
        = lucy_RAMFH_open(NULL, LUCY_FH_READ_ONLY, ram_file);
    lucy_InStream *instream = lucy_InStream_open((cfish_Obj*)file_handle);
    cfish_Obj *self = Cfish_VTable_Foster_Obj(vtable, blank_obj);
    cfish_Obj *deserialized = lucy_Freezer_deserialize(self, instream);

    CHY_UNUSED_VAR(cloning);
    CFISH_DECREF(contents);
    CFISH_DECREF(ram_file);
    CFISH_DECREF(file_handle);
    CFISH_DECREF(instream);

    // Catch bad deserialize() override.
    if (deserialized != self) {
        THROW(CFISH_ERR, "Error when deserializing obj of class %o", klass);
    }
}
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy",
    );
    $binding->append_xs($xs_code);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_cfish_test {
    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Clownfish::Test

bool
run_tests(package)
    char *package;
CODE:
    cfish_CharBuf *class_name = cfish_CB_newf("%s", package);
    cfish_TestFormatter *formatter
        = (cfish_TestFormatter*)cfish_TestFormatterTAP_new();
    bool result = testcfish_Test_run_batch(class_name, formatter);
    CFISH_DECREF(class_name);
    CFISH_DECREF(formatter);

    RETVAL = result;
OUTPUT: RETVAL
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "TestClownfish",
        class_name => "Clownfish::Test",
    );
    $binding->append_xs($xs_code);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_lucy_test {
    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Test

bool
run_tests(package)
    char *package;
CODE:
    cfish_CharBuf *class_name = cfish_CB_newf("%s", package);
    cfish_TestFormatter *formatter
        = (cfish_TestFormatter*)cfish_TestFormatterTAP_new();
    bool result = lucy_Test_run_batch(class_name, formatter);
    CFISH_DECREF(class_name);
    CFISH_DECREF(formatter);

    RETVAL = result;
OUTPUT: RETVAL
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Test",
    );
    $binding->append_xs($xs_code);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_testschema {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Test::TestSchema",
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_bbsortex {
    my @hand_rolled = qw(
        Fetch
        Peek
        Feed
    );
    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy    PACKAGE = Lucy::Test::Util::BBSortEx

SV*
fetch(self)
    lucy_BBSortEx *self;
CODE:
{
    void *address = Lucy_BBSortEx_Fetch(self);
    if (address) {
        RETVAL = XSBind_cfish_to_perl(*(cfish_Obj**)address);
        CFISH_DECREF(*(cfish_Obj**)address);
    }
    else {
        RETVAL = newSV(0);
    }
}
OUTPUT: RETVAL

SV*
peek(self)
    lucy_BBSortEx *self;
CODE:
{
    void *address = Lucy_BBSortEx_Peek(self);
    if (address) {
        RETVAL = XSBind_cfish_to_perl(*(cfish_Obj**)address);
    }
    else {
        RETVAL = newSV(0);
    }
}
OUTPUT: RETVAL

void
feed(self, bb)
    lucy_BBSortEx *self;
    cfish_ByteBuf *bb;
CODE:
    CFISH_INCREF(bb);
    Lucy_BBSortEx_Feed(self, &bb);

END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Test::Util::BBSortEx",
    );
    $binding->exclude_method($_) for @hand_rolled;
    $binding->append_xs($xs_code);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

1;

