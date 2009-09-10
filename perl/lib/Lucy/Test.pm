use Lucy;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Test

void
run_tests(package)
    char *package;
PPCODE:
{
    /* Lucy::Util */
    if (strEQ(package, "TestNumberUtils")) {
        lucy_TestNumUtil_run_tests();
    }
    else if (strEQ(package, "TestStringHelper")) {
        lucy_TestStrHelp_run_tests();
    }
    else {
        warn("Unknown test identifier: '%s'", package);
    }
}
END_XS_CODE

my $charm_xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Test::TestCharmonizer

void
run_tests(which)
    char *which;
PPCODE:
{
    chaz_TestBatch *batch = NULL;
    chaz_Test_init();

    if (strcmp(which, "dirmanip") == 0) {
        batch = chaz_TDirManip_prepare();
    }
    else if (strcmp(which, "integers") == 0) {
        batch = chaz_TIntegers_prepare();
    }
    else if (strcmp(which, "func_macro") == 0) {
        batch = chaz_TFuncMacro_prepare();
    }
    else if (strcmp(which, "headers") == 0) {
        batch = chaz_THeaders_prepare();
    }
    else if (strcmp(which, "large_files") == 0) {
        batch = chaz_TLargeFiles_prepare();
    }
    else if (strcmp(which, "unused_vars") == 0) {
        batch = chaz_TUnusedVars_prepare();
    }
    else if (strcmp(which, "variadic_macros") == 0) {
        batch = chaz_TVariadicMacros_prepare();
    }
    else {
        warn("Unknown test identifier: '%s'", which);
    }

    batch->run_test(batch);
    batch->destroy(batch);
}
END_XS_CODE

Boilerplater::Binding::Perl::Class->register(
    parcel     => "Lucy",
    class_name => "Lucy::Test",
    xs_code    => $xs_code,
);

Boilerplater::Binding::Perl::Class->register(
    parcel     => "Lucy",
    class_name => "Lucy::Test::TestCharmonizer",
    xs_code    => $charm_xs_code,
);

__COPYRIGHT__

    /**
     * Copyright 2006 The Apache Software Foundation
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

