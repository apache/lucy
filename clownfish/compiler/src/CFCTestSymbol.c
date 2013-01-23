/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define CFC_USE_TEST_MACROS
#include "CFCBase.h"
#include "CFCParcel.h"
#include "CFCSymbol.h"
#include "CFCTest.h"

static void
S_run_tests(CFCTest *test);

const CFCTestBatch CFCTEST_BATCH_SYMBOL = {
    "Clownfish::CFC::Model::Symbol",
    28,
    S_run_tests
};

static void
S_run_tests(CFCTest *test) {
    CFCParcel *parcel = CFCParcel_default_parcel();

    {
        static const char *exposures[4] = {
            "public", "private", "parcel", "local"
        };
        static int (*accessors[4])(CFCSymbol *sym) = {
            CFCSymbol_public,
            CFCSymbol_private,
            CFCSymbol_parcel,
            CFCSymbol_local
        };
        for (int i = 0; i < 4; ++i) {
            CFCSymbol *symbol
                = CFCSymbol_new(parcel, exposures[i], NULL, NULL, "sym");
            for (int j = 0; j < 4; ++j) {
                int has_exposure = accessors[j](symbol);
                if (i == j) {
                    OK(test, has_exposure, "exposure %s", exposures[i]);
                }
                else {
                    OK(test, !has_exposure, "%s means not %s", exposures[i],
                       exposures[j]);
                }
            }
            CFCBase_decref((CFCBase*)symbol);
        }
    }

    {
        CFCSymbol *foo = CFCSymbol_new(parcel, "parcel", "Foo", NULL, "sym");
        CFCSymbol *foo_jr
            = CFCSymbol_new(parcel, "parcel", "Foo::FooJr", NULL, "sym");

        int equal = CFCSymbol_equals(foo, foo_jr);
        OK(test, !equal, "different class_name spoils equals");
        const char *foo_jr_name = CFCSymbol_get_class_name(foo_jr);
        STR_EQ(test, foo_jr_name, "Foo::FooJr", "get_class_name");
        const char *foo_jr_cnick = CFCSymbol_get_class_cnick(foo_jr);
        STR_EQ(test, foo_jr_cnick, "FooJr",
               "derive class_cnick from class_name");

        CFCBase_decref((CFCBase*)foo);
        CFCBase_decref((CFCBase*)foo_jr);
    }

    {
        CFCSymbol *public_exposure
            = CFCSymbol_new(parcel, "public", NULL, NULL, "sym");
        CFCSymbol *parcel_exposure
            = CFCSymbol_new(parcel, "parcel", NULL, NULL, "sym");
        int equal = CFCSymbol_equals(public_exposure, parcel_exposure);
        OK(test, !equal, "different exposure spoils equals");
        CFCBase_decref((CFCBase*)public_exposure);
        CFCBase_decref((CFCBase*)parcel_exposure);
    }

    {
        CFCParcel *lucifer_parcel = CFCParcel_new("Lucifer", NULL, NULL);
        CFCParcel_register(lucifer_parcel);
        CFCSymbol *lucifer
            = CFCSymbol_new(lucifer_parcel, "parcel", NULL, NULL, "sym");

        CFCParcel *symbol_parcel = CFCSymbol_get_parcel(lucifer);
        OK(test, symbol_parcel == lucifer_parcel, "derive parcel");
        const char *prefix = CFCSymbol_get_prefix(lucifer);
        STR_EQ(test, prefix, "lucifer_", "get_prefix");
        const char *Prefix = CFCSymbol_get_Prefix(lucifer);
        STR_EQ(test, Prefix, "Lucifer_", "get_Prefix");
        const char *PREFIX = CFCSymbol_get_PREFIX(lucifer);
        STR_EQ(test, PREFIX, "LUCIFER_", "get_PREFIX");

        CFCParcel *luser_parcel = CFCParcel_new("Luser", NULL, NULL);
        CFCParcel_register(luser_parcel);
        CFCSymbol *luser
            = CFCSymbol_new(luser_parcel, "parcel", NULL, NULL, "sym");
        int equal = CFCSymbol_equals(lucifer, luser);
        OK(test, !equal, "different exposure spoils equals");

        CFCBase_decref((CFCBase*)lucifer_parcel);
        CFCBase_decref((CFCBase*)lucifer);
        CFCBase_decref((CFCBase*)luser_parcel);
        CFCBase_decref((CFCBase*)luser);
    }

    {
        CFCSymbol *ooga = CFCSymbol_new(parcel, "parcel", NULL, NULL, "ooga");
        CFCSymbol *booga
            = CFCSymbol_new(parcel, "parcel", NULL, NULL, "booga");
        int equal = CFCSymbol_equals(ooga, booga);
        OK(test, !equal, "different micro_sym spoils equals");
        CFCBase_decref((CFCBase*)ooga);
        CFCBase_decref((CFCBase*)booga);
    }

    {
        CFCParcel *eep_parcel = CFCParcel_new("Eep", NULL, NULL);
        CFCParcel_register(eep_parcel);
        CFCSymbol *eep
            = CFCSymbol_new(eep_parcel, "parcel", "Op::Ork", NULL, "ah_ah");
        const char *short_sym = CFCSymbol_short_sym(eep);
        STR_EQ(test, short_sym, "Ork_ah_ah", "short_sym");
        const char *full_sym = CFCSymbol_full_sym(eep);
        STR_EQ(test, full_sym, "eep_Ork_ah_ah", "full_sym");
        CFCBase_decref((CFCBase*)eep_parcel);
        CFCBase_decref((CFCBase*)eep);
    }

    CFCParcel_reap_singletons();
}

