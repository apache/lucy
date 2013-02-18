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
    $class->bind_test;
    $class->bind_testutils;
    $class->bind_testqueryparsersyntax;
    $class->bind_testschema;
    $class->bind_bbsortex;
}

sub bind_lucy {
    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy    PACKAGE = Lucy

BOOT:
    lucy_Lucy_bootstrap();

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
    lucy_Obj *obj = XSBind_perl_to_cfish(sv);
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
        lucy_Obj* obj = INT2PTR(lucy_Obj*, tmp);
        RETVAL = XSBind_cfish_to_perl(obj);
    }
    else {
        RETVAL = newSVsv(sv);
    }
}
OUTPUT: RETVAL
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
    lucy_CharBuf *class_name = lucy_CB_newf("%s", package);
    lucy_TestFormatter *formatter
        = (lucy_TestFormatter*)lucy_TestFormatterTAP_new();
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

sub bind_testutils {
    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Test::TestUtils

SV*
doc_set()
CODE:
    RETVAL = CFISH_OBJ_TO_SV_NOINC(lucy_TestUtils_doc_set());
OUTPUT: RETVAL
END_XS_CODE
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Test::TestUtils",
    );
    $binding->append_xs($xs_code);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_testqueryparsersyntax {
    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Test::Search::TestQueryParserSyntax

void
run_tests(index);
    lucy_Folder *index;
PPCODE:
    lucy_TestQPSyntax_run_tests(index);
END_XS_CODE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Test::Search::TestQueryParserSyntax",
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
        RETVAL = XSBind_cfish_to_perl(*(lucy_Obj**)address);
        CFISH_DECREF(*(lucy_Obj**)address);
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
        RETVAL = XSBind_cfish_to_perl(*(lucy_Obj**)address);
    }
    else {
        RETVAL = newSV(0);
    }
}
OUTPUT: RETVAL

void
feed(self, bb)
    lucy_BBSortEx *self;
    lucy_ByteBuf *bb;
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

