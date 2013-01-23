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
#include "CFCCBlock.h"
#include "CFCParser.h"
#include "CFCTest.h"

static void
S_run_tests(CFCTest *test);

const CFCTestBatch CFCTEST_BATCH_C_BLOCK = {
    "Clownfish::CFC::Model::CBlock",
    4,
    S_run_tests
};

static void
S_run_tests(CFCTest *test) {
    CFCParser *parser = CFCParser_new();

    {
        CFCCBlock *block = CFCCBlock_new("int foo;");
        STR_EQ(test, CFCCBlock_get_contents(block), "int foo;",
               "get_contents");
        CFCBase_decref((CFCBase*)block);
    }

    {
        const char *cblock_string =
            " __C__\n"
            "#define FOO_BAR 1\n"
            "__END_C__  ";
        CFCBase *result = CFCParser_parse(parser, cblock_string);
        OK(test, result != NULL, "parse cblock");
        STR_EQ(test, CFCBase_get_cfc_class(result),
               "Clownfish::CFC::Model::CBlock", "result class of cblock");
        CFCCBlock *block = (CFCCBlock*)result;
        STR_EQ(test, CFCCBlock_get_contents(block), "#define FOO_BAR 1\n",
               "parse embed_c");
        CFCBase_decref((CFCBase*)block);
    }

    CFCBase_decref((CFCBase*)parser);
}

