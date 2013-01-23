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
#include "CFCFunction.h"
#include "CFCParamList.h"
#include "CFCParcel.h"
#include "CFCParser.h"
#include "CFCTest.h"
#include "CFCType.h"

static void
S_run_tests(CFCTest *test);

const CFCTestBatch CFCTEST_BATCH_FUNCTION = {
    "Clownfish::CFC::Model::Function",
    11,
    S_run_tests
};

static void
S_run_tests(CFCTest *test) {
    CFCParser *parser = CFCParser_new();
    CFCParcel *neato_parcel
        = CFCTest_parse_parcel(test, parser, "parcel Neato;");

    {
        CFCType *return_type = CFCTest_parse_type(test, parser, "Obj*");
        CFCParamList *param_list
            = CFCTest_parse_param_list(test, parser, "(int32_t some_num)");
        CFCFunction *func
            = CFCFunction_new(neato_parcel, NULL, "Neato::Foo", "Foo",
                              "return_an_obj", return_type, param_list,
                              NULL, 0);
        OK(test, func != NULL, "new");

        CFCBase_decref((CFCBase*)return_type);
        CFCBase_decref((CFCBase*)param_list);
        CFCBase_decref((CFCBase*)func);
    }

    {
        CFCParser_set_class_name(parser, "Neato::Obj");
        CFCParser_set_class_cnick(parser, "Obj");
        static const char *func_strings[2] = {
            "inert int running_count(int biscuit);",
            "public inert Hash* init_fave_hash(int32_t num_buckets,"
            " bool o_rly);"
        };
        for (int i = 0; i < 2; ++i) {
            CFCFunction *func
                = CFCTest_parse_function(test, parser, func_strings[i]);
            CFCBase_decref((CFCBase*)func);
        }
    }

    CFCBase_decref((CFCBase*)neato_parcel);
    CFCBase_decref((CFCBase*)parser);

    CFCParcel_reap_singletons();
}

