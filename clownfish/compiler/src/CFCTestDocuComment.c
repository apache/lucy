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
#include "CFCDocuComment.h"
#include "CFCParser.h"
#include "CFCTest.h"
#include "CFCUtil.h"

static void
S_run_tests(CFCTest *test);

const CFCTestBatch CFCTEST_BATCH_DOCU_COMMENT = {
    "Clownfish::CFC::Model::DocuComment",
    15,
    S_run_tests
};

static void
S_run_tests(CFCTest *test) {
    CFCDocuComment *docucomment;

    docucomment = CFCDocuComment_parse("/** foo. */");
    OK(test, docucomment != NULL, "parse");
    CFCBase_decref((CFCBase*)docucomment);

    CFCParser *parser = CFCParser_new();
    const char *text =
        "/**\n"
        " * Brief description.  Long description.\n"
        " *\n"
        " * More long description.\n"
        " *\n"
        " * @param foo A foo.\n"
        " * @param bar A bar.\n"
        " *\n"
        " * @param baz A baz.\n"
        " * @return a return value.\n"
        " */\n";
    CFCBase *result = CFCParser_parse(parser, text);
    OK(test, result != NULL, "parse with CFCParser");
    const char *klass = CFCBase_get_cfc_class(result);
    STR_EQ(test, klass, "Clownfish::CFC::Model::DocuComment", "result class");
    docucomment = (CFCDocuComment*)result;

    const char *brief_desc = CFCDocuComment_get_brief(docucomment);
    const char *brief_expect = "Brief description.";
    STR_EQ(test, brief_desc, brief_expect, "brief description");

    const char *long_desc = CFCDocuComment_get_long(docucomment);
    const char *long_expect =
        "Long description.\n"
        "\n"
        "More long description.";
    STR_EQ(test, long_desc, long_expect, "long description");

    const char *description = CFCDocuComment_get_description(docucomment);
    char *desc_expect = CFCUtil_sprintf("%s  %s", brief_expect, long_expect);
    STR_EQ(test, description, desc_expect, "description");
    FREEMEM(desc_expect);

    const char **param_names = CFCDocuComment_get_param_names(docucomment);
    int num_param_names = 0;
    for (const char **p = param_names; *p; ++p) { ++num_param_names; }
    INT_EQ(test, num_param_names, 3, "number of param names");
    const char *param_names_expect[3] = { "foo", "bar", "baz" };
    for (int i = 0; i < 3; ++i) {
        STR_EQ(test, param_names[i], param_names_expect[i],
               "param name %d", i);
    }

    const char **param_docs = CFCDocuComment_get_param_docs(docucomment);
    int num_param_docs = 0;
    for (const char **p = param_docs; *p; ++p) { ++num_param_docs; }
    INT_EQ(test, num_param_docs, 3, "number of param docs");
    const char *param_docs_expect[3] = { "A foo.", "A bar.", "A baz." };
    const char *param_docs_test[3] = {
        "@param terminated by @",
        "@param terminated by empty line",
        "@param terminated next element, @return"
    };
    for (int i = 0; i < 3; ++i) {
        STR_EQ(test, param_docs[i], param_docs_expect[i], param_docs_test[i]);
    }

    const char *retval = CFCDocuComment_get_retval(docucomment);
    const char *retval_expect = "a return value.";
    STR_EQ(test, retval, retval_expect, "retval");

    CFCBase_decref((CFCBase*)docucomment);
    CFCBase_decref((CFCBase*)parser);
}

