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
#include "CFCClass.h"
#include "CFCMethod.h"
#include "CFCParamList.h"
#include "CFCParcel.h"
#include "CFCParser.h"
#include "CFCSymbol.h"
#include "CFCTest.h"
#include "CFCType.h"
#include "CFCUtil.h"
#include "CFCVariable.h"

static void
S_run_tests(CFCTest *test);

static void
S_test_initial_value(CFCTest *test, CFCParser *parser,
                     const char *const *values, const char *type,
                     const char *test_name);

const CFCTestBatch CFCTEST_BATCH_PARSER = {
    "Clownfish::CFC::Model::Parser",
    189,
    S_run_tests
};

static void
S_run_tests(CFCTest *test) {
    CFCParser *parser = CFCParser_new();
    OK(test, parser != NULL, "new");

    {
        CFCParcel *fish = CFCTest_parse_parcel(test, parser, "parcel Fish;");

        CFCParcel *registered = CFCParcel_new("Crustacean", "Crust", NULL);
        CFCParcel_register(registered);
        CFCParcel *parcel
            = CFCTest_parse_parcel(test, parser, "parcel Crustacean;");
        OK(test, parcel == registered, "Fetch registered parcel");
        OK(test, CFCParser_get_parcel(parser) == parcel,
           "parcel_definition sets internal var");

        CFCBase_decref((CFCBase*)fish);
        CFCBase_decref((CFCBase*)registered);
        CFCBase_decref((CFCBase*)parcel);
    }

    {
        static const char *const specifiers[8] = {
            "foo", "_foo", "foo_yoo", "FOO", "Foo", "fOO", "f00", "foo_foo_foo"
        };
        for (int i = 0; i < 8; ++i) {
            const char *specifier = specifiers[i];
            char *src = CFCUtil_sprintf("int32_t %s;", specifier);
            CFCVariable *var = CFCTest_parse_variable(test, parser, src);
            STR_EQ(test, CFCVariable_micro_sym(var), specifier,
                   "identifier/declarator: %s", specifier);
            FREEMEM(src);
            CFCBase_decref((CFCBase*)var);
        }
    }

    {
        static const char *const specifiers[6] = {
            "void", "float", "uint32_t", "int64_t", "uint8_t", "bool"
        };
        for (int i = 0; i < 6; ++i) {
            const char *specifier = specifiers[i];
            char *src = CFCUtil_sprintf("int32_t %s;", specifier);
            CFCBase *result = CFCParser_parse(parser, src);
            OK(test, result == NULL,
               "reserved word not parsed as identifier: %s", specifier);
            FREEMEM(src);
            CFCBase_decref(result);
        }
    }

    {
        static const char *const type_strings[7] = {
            "bool", "const char *", "Obj*", "i32_t", "char[]", "long[1]",
            "i64_t[30]"
        };
        for (int i = 0; i < 7; ++i) {
            const char *type_string = type_strings[i];
            CFCType *type = CFCTest_parse_type(test, parser, type_string);
            CFCBase_decref((CFCBase*)type);
        }
    }

    {
        static const char *const class_names[7] = {
            "ByteBuf", "Obj", "ANDMatcher", "Foo", "FooJr", "FooIII", "Foo4th"
        };
        for (int i = 0; i < 7; ++i) {
            const char *class_name = class_names[i];
            char *src      = CFCUtil_sprintf("%s*", class_name);
            char *expected = CFCUtil_sprintf("crust_%s", class_name);
            CFCType *type = CFCTest_parse_type(test, parser, src);
            STR_EQ(test, CFCType_get_specifier(type), expected,
                   "object_type_specifier: %s", class_name);
            FREEMEM(src);
            FREEMEM(expected);
            CFCBase_decref((CFCBase*)type);
        }
    }

    {
        CFCType *type = CFCTest_parse_type(test, parser, "const char");
        OK(test, CFCType_const(type), "type_qualifier const");
        CFCBase_decref((CFCBase*)type);
    }

    {
        static const char *const exposures[2] = {
            "public", ""
        };
        static int (*const accessors[2])(CFCSymbol *sym) = {
            CFCSymbol_public, CFCSymbol_parcel
        };
        for (int i = 0; i < 2; ++i) {
            const char *exposure = exposures[i];
            char *src = CFCUtil_sprintf("%s inert int32_t foo;", exposure);
            CFCVariable *var = CFCTest_parse_variable(test, parser, src);
            OK(test, accessors[i]((CFCSymbol*)var), "exposure_specifier %s",
               exposure);
            FREEMEM(src);
            CFCBase_decref((CFCBase*)var);
        }
    }

    {
        static const char *const hex_constants[] = {
            "0x1", "0x0a", "0xFFFFFFFF", "-0xFC", NULL
        };
        S_test_initial_value(test, parser, hex_constants, "int32_t",
                             "hex_constant:");
    }

    {
        static const char *const integer_constants[] = {
            "1", "-9999", "0", "10000", NULL
        };
        S_test_initial_value(test, parser, integer_constants, "int32_t",
                             "integer_constant:");
    }

    {
        static const char *const float_constants[] = {
            "1.0", "-9999.999", "0.1", "0.0", NULL
        };
        S_test_initial_value(test, parser, float_constants, "double",
                             "float_constant:");
    }

    {
        static const char *const string_literals[] = {
            "\"blah\"", "\"blah blah\"", "\"\\\"blah\\\" \\\"blah\\\"\"", NULL
        };
        S_test_initial_value(test, parser, string_literals, "CharBuf*",
                             "string_literal:");
    }

    {
        static const char *const composites[5] = {
            "int[]", "i32_t **", "Foo **", "Foo ***", "const void *"
        };
        for (int i = 0; i < 5; ++i) {
            const char *composite = composites[i];
            CFCType *type = CFCTest_parse_type(test, parser, composite);
            OK(test, CFCType_is_composite(type), "composite_type: %s",
               composite);
            CFCBase_decref((CFCBase*)type);
        }
    }

    {
        static const char *const object_types[5] = {
            "Obj *", "incremented Foo*", "decremented CharBuf *"
        };
        for (int i = 0; i < 3; ++i) {
            const char *object_type = object_types[i];
            CFCType *type = CFCTest_parse_type(test, parser, object_type);
            OK(test, CFCType_is_object(type), "object_type: %s",
               object_type);
            CFCBase_decref((CFCBase*)type);
        }
    }

    {
        static const char *const param_list_strings[3] = {
            "()",
            "(int foo)",
            "(Obj *foo, Foo **foo_ptr)"
        };
        for (int i = 0; i < 3; ++i) {
            const char *param_list_string = param_list_strings[i];
            CFCParamList *param_list
                = CFCTest_parse_param_list(test, parser, param_list_string);
            INT_EQ(test, CFCParamList_num_vars(param_list), i,
                   "param list num_vars: %d", i);
            CFCBase_decref((CFCBase*)param_list);
        }
    }

    {
        CFCParamList *param_list
            = CFCTest_parse_param_list(test, parser, "(int foo, ...)");
        OK(test, CFCParamList_variadic(param_list), "variadic param list");
        CFCBase_decref((CFCBase*)param_list);
    }

    {
        const char *param_list_string =
            "(int foo = 0xFF, char *bar =\"blah\")";
        CFCParamList *param_list
            = CFCTest_parse_param_list(test, parser, param_list_string);
        const char **initial_values
            = CFCParamList_get_initial_values(param_list);
        STR_EQ(test, initial_values[0], "0xFF",
               "param list initial_values[0]");
        STR_EQ(test, initial_values[1], "\"blah\"",
               "param list initial_values[1]");
        OK(test, initial_values[2] == NULL, "param list initial_values[2]");
        CFCBase_decref((CFCBase*)param_list);
    }

    {
        CFCParser_set_class_name(parser, "Stuff::Obj");
        CFCParser_set_class_cnick(parser, "Obj");

        const char *method_string =
            "public Foo* Spew_Foo(Obj *self, uint32_t *how_many);";
        CFCMethod *method = CFCTest_parse_method(test, parser, method_string);
        CFCBase_decref((CFCBase*)method);

        const char *var_string =
            "public inert Hash *hash;";
        CFCVariable *var = CFCTest_parse_variable(test, parser, var_string);
        CFCBase_decref((CFCBase*)var);
    }

    {
        static const char *const class_names[4] = {
            "Foo", "Foo::FooJr", "Foo::FooJr::FooIII",
            "Foo::FooJr::FooIII::Foo4th"
        };
        for (int i = 0; i < 4; ++i) {
            const char *class_name = class_names[i];
            char *class_string = CFCUtil_sprintf("class %s { }", class_name);
            CFCClass *klass
                = CFCTest_parse_class(test, parser, class_string);
            STR_EQ(test, CFCClass_get_class_name(klass), class_name,
                   "class_name: %s", class_name);
            FREEMEM(class_string);
            CFCBase_decref((CFCBase*)klass);
        }
    }

    {
        static const char *const cnicks[2] =  { "Foo", "FF" };
        for (int i = 0; i < 2; ++i) {
            const char *cnick = cnicks[i];
            char *class_string
                = CFCUtil_sprintf("class Foodie%s cnick %s { }", cnick, cnick);
            CFCClass *klass
                = CFCTest_parse_class(test, parser, class_string);
            STR_EQ(test, CFCClass_get_cnick(klass), cnick, "cnick: %s", cnick);
            FREEMEM(class_string);
            CFCBase_decref((CFCBase*)klass);
        }
    }

    CFCBase_decref((CFCBase*)parser);

    CFCClass_clear_registry();
    CFCParcel_reap_singletons();
}

static void
S_test_initial_value(CFCTest *test, CFCParser *parser,
                     const char *const *values, const char *type,
                     const char *test_name) {
    for (int i = 0; values[i]; ++i) {
        const char *value = values[i];
        char *src = CFCUtil_sprintf("(%s foo = %s)", type, value);
        CFCParamList *param_list
            = CFCTest_parse_param_list(test, parser, src);
        const char **initial_values
            = CFCParamList_get_initial_values(param_list);
        STR_EQ(test, initial_values[0], value, "%s %s", test_name, value);
        FREEMEM(src);
        CFCBase_decref((CFCBase*)param_list);
    }
}

