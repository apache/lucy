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
    my ($class, $hierarchy) = @_;
    $class->inherit_metadata($hierarchy);
    $class->bind_lucy;
    $class->bind_test;
}

sub inherit_metadata {
    my ($class, $hierarchy) = @_;

    require Clownfish;

    for my $class (@{ $hierarchy->ordered_classes }) {
        next if $class->get_parcel->get_name ne 'Clownfish' || $class->inert;

        my $class_name = $class->get_class_name;
        my $vtable     = Clownfish::VTable->fetch_vtable($class_name);

        for my $rt_method (@{ $vtable->get_methods }) {
            if ($rt_method->is_excluded_from_host) {
                my $method = $class->method($rt_method->get_name);
                $method->exclude_from_host;
            }
            else {
                my $alias = $rt_method->get_host_alias;
                if (defined($alias)) {
                    my $method = $class->method($rt_method->get_name);
                    $method->set_host_alias($alias);
                }
            }
        }
    }
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
    CFISH_UNUSED_VAR(self);
    if (items < 2 || !SvTRUE(ST(1))) {
        SV *retval;
        cfish_ByteBuf *serialized_bb;
        lucy_RAMFileHandle *file_handle
            = lucy_RAMFH_open(NULL, LUCY_FH_WRITE_ONLY | LUCY_FH_CREATE, NULL);
        lucy_OutStream *target = lucy_OutStream_open((cfish_Obj*)file_handle);

        lucy_Freezer_serialize(self, target);

        LUCY_OutStream_Close(target);
        serialized_bb
            = LUCY_RAMFile_Get_Contents(LUCY_RAMFH_Get_File(file_handle));
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
    cfish_Obj *self = CFISH_VTable_Foster_Obj(vtable, blank_obj);
    cfish_Obj *deserialized = lucy_Freezer_deserialize(self, instream);

    CFISH_UNUSED_VAR(cloning);
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

sub bind_test {
    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Test

bool
run_tests(package)
    char *package;
CODE:
    cfish_CharBuf *class_name = cfish_CB_newf("%s", package);
    cfish_TestFormatter *formatter
        = (cfish_TestFormatter*)cfish_TestFormatterTAP_new();
    cfish_TestSuite *suite = testlucy_Test_create_test_suite();
    bool result = CFISH_TestSuite_Run_Batch(suite, class_name, formatter);
    CFISH_DECREF(class_name);
    CFISH_DECREF(formatter);
    CFISH_DECREF(suite);

    RETVAL = result;
OUTPUT: RETVAL
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "TestLucy",
        class_name => "Lucy::Test",
    );
    $binding->append_xs($xs_code);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

1;

