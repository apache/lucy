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
#include "CFCParser.h"
#include "CFCSymbol.h"
#include "CFCTest.h"
#include "CFCType.h"
#include "CFCVariable.h"

static void
S_run_tests(CFCTest *test);

const CFCTestBatch CFCTEST_BATCH_VARIABLE = {
    "Clownfish::CFC::Model::Variable",
    27,
    S_run_tests
};

static void
S_run_tests(CFCTest *test) {
    CFCParser *parser = CFCParser_new();
    CFCParcel *neato_parcel
        = CFCTest_parse_parcel(test, parser, "parcel Neato;");

    {
        CFCType *type = CFCTest_parse_type(test, parser, "float*");
        CFCVariable *var
            = CFCVariable_new(NULL, NULL, NULL, NULL, "foo", type, 0);
        STR_EQ(test, CFCVariable_local_c(var), "float* foo", "local_c");
        STR_EQ(test, CFCVariable_local_declaration(var), "float* foo;",
               "local_declaration");
        OK(test, CFCSymbol_local((CFCSymbol*)var), "default to local access");

        CFCBase_decref((CFCBase*)type);
        CFCBase_decref((CFCBase*)var);
    }

    {
        CFCType *type = CFCTest_parse_type(test, parser, "float[1]");
        CFCVariable *var
            = CFCVariable_new(NULL, NULL, NULL, NULL, "foo", type, 0);
        STR_EQ(test, CFCVariable_local_c(var), "float foo[1]",
               "to_c appends array to var name rather than type specifier");

        CFCBase_decref((CFCBase*)type);
        CFCBase_decref((CFCBase*)var);
    }

    {
        CFCType *type = CFCTest_parse_type(test, parser, "Foo*");
        CFCVariable *var
            = CFCVariable_new(neato_parcel, NULL,
                              "Crustacean::Lobster::LobsterClaw", "LobClaw",
                              "foo", type, 0);
        STR_EQ(test, CFCVariable_global_c(var), "neato_Foo* neato_LobClaw_foo",
               "global_c");

        CFCBase_decref((CFCBase*)type);
        CFCBase_decref((CFCBase*)var);
    }

    {
        static const char *variable_strings[7] = {
            "int foo;",
            "inert Obj *obj;",
            "public inert int32_t **foo;",
            "Dog *fido;",
            "uint32_t baz",
            "CharBuf *stuff",
            "float **ptr"
        };
        for (int i = 0; i < 7; ++i) {
            CFCVariable *var
                = CFCTest_parse_variable(test, parser, variable_strings[i]);
            CFCBase_decref((CFCBase*)var);
        }
    }

    CFCBase_decref((CFCBase*)parser);
    CFCBase_decref((CFCBase*)neato_parcel);

    CFCParcel_reap_singletons();
}

