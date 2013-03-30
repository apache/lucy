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
#include "CFCParamList.h"
#include "CFCParcel.h"
#include "CFCParser.h"
#include "CFCTest.h"
#include "CFCVariable.h"

static void
S_run_tests(CFCTest *test);

const CFCTestBatch CFCTEST_BATCH_PARAM_LIST = {
    "Clownfish::CFC::Model::ParamList",
    21,
    S_run_tests
};

static void
S_run_tests(CFCTest *test) {
    CFCParser *parser = CFCParser_new();
    CFCParcel *neato_parcel
        = CFCTest_parse_parcel(test, parser, "parcel Neato;");

    {
        CFCParamList *param_list
            = CFCTest_parse_param_list(test, parser, "(Obj *self, int num)");
        OK(test, !CFCParamList_variadic(param_list), "not variadic");
        STR_EQ(test, CFCParamList_to_c(param_list), "neato_Obj* self, int num",
               "to_c");
        STR_EQ(test, CFCParamList_name_list(param_list), "self, num",
               "name_list");

        CFCBase_decref((CFCBase*)param_list);
    }

    {
        CFCParamList *param_list
            = CFCTest_parse_param_list(test, parser,
                                       "(Obj *self=NULL, int num, ...)");
        OK(test, CFCParamList_variadic(param_list), "variadic");
        STR_EQ(test, CFCParamList_to_c(param_list),
               "neato_Obj* self, int num, ...", "to_c");
        INT_EQ(test, CFCParamList_num_vars(param_list), 2, "num_vars");
        const char **initial_values
            = CFCParamList_get_initial_values(param_list);
        STR_EQ(test, initial_values[0], "NULL", "initial_values[0]"); 
        OK(test, initial_values[1] == NULL, "initial_values[1]");
        CFCVariable **variables = CFCParamList_get_variables(param_list);
        OK(test, variables[0] != NULL, "get_variables");
        STR_EQ(test, CFCBase_get_cfc_class((CFCBase*)variables[0]),
               "Clownfish::CFC::Model::Variable", "get_variables return type");

        CFCBase_decref((CFCBase*)param_list);
    }

    {
        CFCParamList *param_list
            = CFCTest_parse_param_list(test, parser, "()");
        STR_EQ(test, CFCParamList_to_c(param_list), "void", "to_c");
        INT_EQ(test, CFCParamList_num_vars(param_list), 0, "num_vars");
        CFCVariable **variables = CFCParamList_get_variables(param_list);
        OK(test, variables[0] == NULL, "get_variables");

        CFCBase_decref((CFCBase*)param_list);
    }

    CFCBase_decref((CFCBase*)parser);
    CFCBase_decref((CFCBase*)neato_parcel);

    CFCParcel_reap_singletons();
}

