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

#include "charmony.h"

#define CFC_USE_TEST_MACROS
#include "CFCBase.h"
#include "CFCParcel.h"
#include "CFCSymbol.h"
#include "CFCVersion.h"
#include "CFCTest.h"

static void
S_run_tests(CFCTest *test);

const CFCTestBatch CFCTEST_BATCH_PARCEL = {
    "Clownfish::CFC::Model::Parcel",
    10,
    S_run_tests
};

static void
S_run_tests(CFCTest *test) {
    {
        CFCParcel *parcel = CFCParcel_new("Foo", NULL, NULL);
        OK(test, parcel != NULL, "new");
        CFCBase_decref((CFCBase*)parcel);
    }

    {
        const char *json =
            "        {\n"
            "            \"name\": \"Crustacean\",\n"
            "            \"nickname\": \"Crust\",\n"
            "            \"version\": \"v0.1.0\"\n"
            "        }\n";
        CFCParcel *parcel = CFCParcel_new_from_json(json);
        OK(test, parcel != NULL, "new_from_json");
        CFCBase_decref((CFCBase*)parcel);
    }

    {
        const char *path = "t" CHY_DIR_SEP "cfsource" CHY_DIR_SEP "Animal.cfp";
        CFCParcel *parcel = CFCParcel_new_from_file(path);
        OK(test, parcel != NULL, "new_from_file");
        CFCBase_decref((CFCBase*)parcel);
    }

    {
        CFCParcel *parcel = CFCParcel_default_parcel();
        CFCSymbol *thing = CFCSymbol_new(parcel, "parcel", NULL, NULL, "sym");
        STR_EQ(test, CFCSymbol_get_prefix(thing), "",
               "get_prefix with no parcel");
        STR_EQ(test, CFCSymbol_get_Prefix(thing), "",
               "get_Prefix with no parcel");
        STR_EQ(test, CFCSymbol_get_PREFIX(thing), "",
               "get_PREFIX with no parcel");
        CFCBase_decref((CFCBase*)thing);
    }

    {
        CFCParcel *parcel = CFCParcel_new("Crustacean", "Crust", NULL);
        CFCParcel_register(parcel);
        STR_EQ(test, CFCVersion_get_vstring(CFCParcel_get_version(parcel)),
               "v0", "get_version");

        CFCSymbol *thing = CFCSymbol_new(parcel, "parcel", NULL, NULL, "sym");
        STR_EQ(test, CFCSymbol_get_prefix(thing), "crust_",
               "get_prefix with parcel");
        STR_EQ(test, CFCSymbol_get_Prefix(thing), "Crust_",
               "get_Prefix with parcel");
        STR_EQ(test, CFCSymbol_get_PREFIX(thing), "CRUST_",
               "get_PREFIX with parcel");

        CFCBase_decref((CFCBase*)thing);
        CFCBase_decref((CFCBase*)parcel);
    }

    CFCParcel_reap_singletons();
}

