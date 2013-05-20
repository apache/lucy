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
#include "CFCParcel.h"
#include "CFCParser.h"
#include "CFCTest.h"
#include "CFCType.h"
#include "CFCUtil.h"

#ifndef true
  #define true 1
  #define false 0
#endif

static void
S_run_tests(CFCTest *test);

static void
S_run_basic_tests(CFCTest *test);

static void
S_run_primitive_tests(CFCTest *test);

static void
S_run_integer_tests(CFCTest *test);

static void
S_run_float_tests(CFCTest *test);

static void
S_run_void_tests(CFCTest *test);

static void
S_run_object_tests(CFCTest *test);

static void
S_run_va_list_tests(CFCTest *test);

static void
S_run_arbitrary_tests(CFCTest *test);

static void
S_run_composite_tests(CFCTest *test);

const CFCTestBatch CFCTEST_BATCH_TYPE = {
    "Clownfish::CFC::Model::Type",
    360,
    S_run_tests
};

static void
S_run_tests(CFCTest *test) {
    S_run_basic_tests(test);
    S_run_primitive_tests(test);
    S_run_integer_tests(test);
    S_run_float_tests(test);
    S_run_void_tests(test);
    S_run_object_tests(test);
    S_run_va_list_tests(test);
    S_run_arbitrary_tests(test);
    S_run_composite_tests(test);
}

static void
S_run_basic_tests(CFCTest *test) {
    CFCParcel *neato_parcel = CFCParcel_new("Neato", NULL, NULL, false);
    CFCParcel_register(neato_parcel);
    CFCType *type = CFCType_new(0, neato_parcel, "mytype_t", 0);

    OK(test, CFCType_get_parcel(type) == neato_parcel, "get_parcel");
    STR_EQ(test, CFCType_to_c(type), "mytype_t", "to_c");
    STR_EQ(test, CFCType_get_specifier(type), "mytype_t", "get_specifier");

#define TEST_BOOL_ACCESSOR(type, name) \
    OK(test, !CFCType_ ## name(type), #name " false by default");

    TEST_BOOL_ACCESSOR(type, const);
    TEST_BOOL_ACCESSOR(type, nullable);
    TEST_BOOL_ACCESSOR(type, incremented);
    TEST_BOOL_ACCESSOR(type, decremented);
    TEST_BOOL_ACCESSOR(type, is_void);
    TEST_BOOL_ACCESSOR(type, is_object);
    TEST_BOOL_ACCESSOR(type, is_primitive);
    TEST_BOOL_ACCESSOR(type, is_integer);
    TEST_BOOL_ACCESSOR(type, is_floating);
    TEST_BOOL_ACCESSOR(type, is_string_type);
    TEST_BOOL_ACCESSOR(type, is_va_list);
    TEST_BOOL_ACCESSOR(type, is_arbitrary);
    TEST_BOOL_ACCESSOR(type, is_composite);

    CFCBase_decref((CFCBase*)neato_parcel);
    CFCBase_decref((CFCBase*)type);

    CFCParcel_reap_singletons();
}

static void
S_run_primitive_tests(CFCTest *test) {
    CFCParcel *parcel = CFCParcel_default_parcel();
    CFCType *type = CFCType_new(CFCTYPE_PRIMITIVE, parcel, "hump_t", 0);
    OK(test, CFCType_is_primitive(type), "is_primitive");

    {
        CFCType *twin = CFCType_new(CFCTYPE_PRIMITIVE, parcel, "hump_t", 0);
        OK(test, CFCType_equals(type, twin), "equals");
        CFCBase_decref((CFCBase*)twin);
    }

    {
        CFCType *other = CFCType_new(CFCTYPE_PRIMITIVE, parcel, "dump_t", 0);
        OK(test, !CFCType_equals(type, other), "equals spoiled by specifier");
        CFCBase_decref((CFCBase*)other);
    }

    {
        CFCType *other = CFCType_new(CFCTYPE_PRIMITIVE|CFCTYPE_CONST, parcel,
                                     "hump_t", 0);
        OK(test, !CFCType_equals(type, other), "equals spoiled by const");
        CFCBase_decref((CFCBase*)other);
    }

    CFCBase_decref((CFCBase*)type);
}

static void
S_run_integer_tests(CFCTest *test) {
    {
        CFCType *type = CFCType_new_integer(CFCTYPE_CONST, "int32_t");
        OK(test, CFCType_const(type), "const");
        STR_EQ(test, CFCType_get_specifier(type), "int32_t", "get_specifier");
        STR_EQ(test, CFCType_to_c(type), "const int32_t",
               "'const' in C representation");
        CFCBase_decref((CFCBase*)type);
    }

    {
        CFCParser *parser = CFCParser_new();
        static const char *specifiers[14] = {
            "bool",
            "char",
            "short",
            "int",
            "long",
            "size_t",
            "int8_t",
            "int16_t",
            "int32_t",
            "int64_t",
            "uint8_t",
            "uint16_t",
            "uint32_t",
            "uint64_t"
        };
        for (int i = 0; i < 14; ++i) {
            const char *specifier = specifiers[i];
            CFCType *type;

            type = CFCTest_parse_type(test, parser, specifier);
            OK(test, CFCType_is_integer(type), "%s is_integer", specifier);
            CFCBase_decref((CFCBase*)type);

            char *const_specifier = CFCUtil_sprintf("const %s", specifier);
            type = CFCTest_parse_type(test, parser, const_specifier);
            OK(test, CFCType_is_integer(type), "%s is_integer",
               const_specifier);
            OK(test, CFCType_const(type), "%s is const", const_specifier);
            FREEMEM(const_specifier);
            CFCBase_decref((CFCBase*)type);
        }
        CFCBase_decref((CFCBase*)parser);
    }
}

static void
S_run_float_tests(CFCTest *test) {
    {
        CFCType *type = CFCType_new_float(CFCTYPE_CONST, "float");
        OK(test, CFCType_const(type), "const");
        STR_EQ(test, CFCType_get_specifier(type), "float", "get_specifier");
        STR_EQ(test, CFCType_to_c(type), "const float",
               "'const' in C representation");
        CFCBase_decref((CFCBase*)type);
    }

    {
        CFCParser *parser = CFCParser_new();
        static const char *specifiers[2] = {
            "float",
            "double"
        };
        for (int i = 0; i < 2; ++i) {
            const char *specifier = specifiers[i];
            CFCType *type;

            type = CFCTest_parse_type(test, parser, specifier);
            OK(test, CFCType_is_floating(type), "%s is_floating", specifier);
            CFCBase_decref((CFCBase*)type);

            char *const_specifier = CFCUtil_sprintf("const %s", specifier);
            type = CFCTest_parse_type(test, parser, const_specifier);
            OK(test, CFCType_is_floating(type), "%s is_floating",
               const_specifier);
            OK(test, CFCType_const(type), "%s is const", const_specifier);
            FREEMEM(const_specifier);
            CFCBase_decref((CFCBase*)type);
        }
        CFCBase_decref((CFCBase*)parser);
    }
}

static void
S_run_void_tests(CFCTest *test) {
    CFCParser *parser = CFCParser_new();

    {
        CFCType *type = CFCType_new_void(false);
        STR_EQ(test, CFCType_get_specifier(type), "void", "get_specifier");
        STR_EQ(test, CFCType_to_c(type), "void", "to_c");
        OK(test, CFCType_is_void(type), "is_void");
        CFCBase_decref((CFCBase*)type);
    }

    {
        CFCType *type = CFCType_new_void(true);
        STR_EQ(test, CFCType_to_c(type), "const void",
               "'const' in C representation");
        CFCBase_decref((CFCBase*)type);
    }

    {
        CFCType *type = CFCTest_parse_type(test, parser, "void");
        OK(test, CFCType_is_void(type), "void is_void");
        CFCBase_decref((CFCBase*)type);
    }

    {
        CFCType *type = CFCTest_parse_type(test, parser, "const void");
        OK(test, CFCType_is_void(type), "const void is_void");
        OK(test, CFCType_const(type), "const void is const");
        CFCBase_decref((CFCBase*)type);
    }

    CFCBase_decref((CFCBase*)parser);
}

static void
S_run_object_tests(CFCTest *test) {
    static const char *modifiers[4] = {
        "const", "incremented", "decremented", "nullable"
    };
    static int flags[4] = {
        CFCTYPE_CONST,
        CFCTYPE_INCREMENTED,
        CFCTYPE_DECREMENTED,
        CFCTYPE_NULLABLE
    };
    static int (*accessors[4])(CFCType *type) = {
        CFCType_const,
        CFCType_incremented,
        CFCType_decremented,
        CFCType_nullable
    };

    {
        CFCParser *parser = CFCParser_new();
        CFCParcel *neato_parcel
            = CFCTest_parse_parcel(test, parser, "parcel Neato;");

        static const char *specifiers[4] = {
            "Foo", "FooJr", "FooIII", "Foo4th"
        };
        for (int i = 0; i < 4; ++i) {
            const char *specifier = specifiers[i];

            char *class_code = CFCUtil_sprintf("class %s {}", specifier);
            CFCClass *klass = CFCTest_parse_class(test, parser, class_code);
            CFCClass *class_list[2] = { klass, NULL };
            FREEMEM(class_code);

            static const char *prefixes[2] = { "", "neato_" };
            char *expect = CFCUtil_sprintf("neato_%s", specifier);
            for (int j = 0; j < 2; ++j) {
                char *src = CFCUtil_sprintf("%s%s*", prefixes[j], specifier);
                CFCType *type = CFCTest_parse_type(test, parser, src);
                CFCType_resolve(type, class_list);
                STR_EQ(test, CFCType_get_specifier(type), expect,
                       "object_type_specifier: %s", src);
                OK(test, CFCType_is_object(type), "%s is_object", src);
                INT_EQ(test, CFCType_get_indirection(type), 1,
                       "%s indirection", src);

                FREEMEM(src);
                CFCBase_decref((CFCBase*)type);
            }
            FREEMEM(expect);

            for (int j = 0; j < 4; ++j) {
                char *src = CFCUtil_sprintf("%s %s*", modifiers[j], specifier);
                CFCType *type = CFCTest_parse_type(test, parser, src);
                OK(test, CFCType_is_object(type), "%s is_object", src);
                OK(test, accessors[j](type), "%s accessor", src);

                FREEMEM(src);
                CFCBase_decref((CFCBase*)type);
            }

            CFCBase_decref((CFCBase*)klass);
            CFCClass_clear_registry();
        }

        CFCBase_decref((CFCBase*)neato_parcel);
        CFCBase_decref((CFCBase*)parser);
    }

    CFCParcel *neato_parcel = CFCParcel_new("Neato", NULL, NULL, false);
    CFCClass *foo_class
        = CFCClass_create(neato_parcel, NULL, "Foo", NULL, NULL, NULL, NULL,
                          NULL, false, false);
    CFCClass *class_list[2] = { foo_class, NULL };
    CFCType *foo = CFCType_new_object(0, NULL, "Foo", 1);
    CFCType_resolve(foo, class_list);

    {
        CFCType *another_foo = CFCType_new_object(0, NULL, "Foo", 1);
        CFCType_resolve(another_foo, class_list);
        OK(test, CFCType_equals(foo, another_foo), "equals");
        CFCBase_decref((CFCBase*)another_foo);
    }

    {
        CFCType *bar = CFCType_new_object(0, NULL, "Bar", 1);
        OK(test, !CFCType_equals(foo, bar),
           "different specifier spoils equals");
        CFCBase_decref((CFCBase*)bar);
    }

    {
        CFCParcel *foreign_parcel
            = CFCParcel_new("Foreign", NULL, NULL, false);
        CFCClass *foreign_foo_class
            = CFCClass_create(foreign_parcel, NULL, "Foo", NULL, NULL, NULL,
                              NULL, NULL, false, false);
        CFCClass *foreign_class_list[2] = { foreign_foo_class, NULL };
        CFCType *foreign_foo = CFCType_new_object(0, foreign_parcel, "Foo", 1);
        CFCType_resolve(foreign_foo, foreign_class_list);
        OK(test, !CFCType_equals(foo, foreign_foo),
           "different parcel spoils equals");
        STR_EQ(test, CFCType_get_specifier(foreign_foo), "foreign_Foo",
               "prepend parcel prefix to specifier");
        CFCBase_decref((CFCBase*)foreign_parcel);
        CFCBase_decref((CFCBase*)foreign_foo_class);
        CFCBase_decref((CFCBase*)foreign_foo);
    }

    {
        for (int i = 0; i < 4; ++i) {
            CFCType *modified_foo
                = CFCType_new_object(flags[i], NULL, "Foo", 1);
            OK(test, accessors[i](modified_foo), "%s", modifiers[i]);
            OK(test, !accessors[i](foo), "not %s", modifiers[i]);
            OK(test, !CFCType_equals(foo, modified_foo),
               "different %s spoils equals", modifiers[i]);
            OK(test, !CFCType_similar(foo, modified_foo),
               "different %s spoils similar", modifiers[i]);
            CFCBase_decref((CFCBase*)modified_foo);
        }
    }

    {
        CFCType *string_type = CFCType_new_object(0, NULL, "CharBuf", 1);
        OK(test, CFCType_is_string_type(string_type), "%s", "is_string_type");
        OK(test, !CFCType_is_string_type(foo), "not %s", "not is_string_type");
        CFCBase_decref((CFCBase*)string_type);
    }

    CFCBase_decref((CFCBase*)neato_parcel);
    CFCBase_decref((CFCBase*)foo_class);
    CFCBase_decref((CFCBase*)foo);

    CFCClass_clear_registry();
    CFCParcel_reap_singletons();
}

static void
S_run_va_list_tests(CFCTest *test) {
    {
        CFCType *type = CFCType_new_va_list();
        STR_EQ(test, CFCType_get_specifier(type), "va_list",
               "specifier defaults to 'va_list'");
        STR_EQ(test, CFCType_to_c(type), "va_list", "to_c");
        CFCBase_decref((CFCBase*)type);
    }

    {
        CFCParser *parser = CFCParser_new();
        CFCType *type = CFCTest_parse_type(test, parser, "va_list");
        OK(test, CFCType_is_va_list(type), "is_va_list");
        CFCBase_decref((CFCBase*)type);
        CFCBase_decref((CFCBase*)parser);
    }
}

static void
S_run_arbitrary_tests(CFCTest *test) {
    {
        CFCParcel *neato_parcel = CFCParcel_new("Neato", NULL, NULL, false);
        CFCParcel_register(neato_parcel);

        CFCType *foo = CFCType_new_arbitrary(neato_parcel, "foo_t");
        STR_EQ(test, CFCType_get_specifier(foo), "foo_t", "get_specifier");
        STR_EQ(test, CFCType_to_c(foo), "foo_t", "to_c");

        CFCType *twin = CFCType_new_arbitrary(neato_parcel, "foo_t");
        OK(test, CFCType_equals(foo, twin), "equals");

        CFCType *compare_t
            = CFCType_new_arbitrary(neato_parcel, "Sort_compare_t");
        OK(test, !CFCType_equals(foo, compare_t),
           "equals spoiled by different specifier");

        CFCBase_decref((CFCBase*)neato_parcel);
        CFCBase_decref((CFCBase*)foo);
        CFCBase_decref((CFCBase*)compare_t);
        CFCBase_decref((CFCBase*)twin);
    }

    {
        CFCParser *parser = CFCParser_new();
        static const char *specifiers[2] = { "foo_t", "Sort_compare_t" };
        for (int i = 0; i < 2; ++i) {
            const char *specifier = specifiers[i];
            CFCType *type = CFCTest_parse_type(test, parser, specifier);
            OK(test, CFCType_is_arbitrary(type), "arbitrary type %s",
               specifier);
            CFCBase_decref((CFCBase*)type);
        }
        CFCBase_decref((CFCBase*)parser);
    }

    CFCParcel_reap_singletons();
}

static void
S_run_composite_tests(CFCTest *test) {
    CFCParser *parser = CFCParser_new();

    {
        static const char *type_strings[14] = {
            "char*",
            "char**",
            "char***",
            "int32_t*",
            "Obj**",
            "int8_t[]",
            "int8_t[1]",
            "neato_method_t[]",
            "neato_method_t[1]",
            "multi_dimensional_t[1][10]",
            "char * * ",
            "const Obj**",
            "const void*",
            "int8_t[ 3 ]"
        };
        for (int i = 0; i < 14; ++i) {
            const char *type_string = type_strings[i];
            CFCType *type = CFCTest_parse_type(test, parser, type_string);
            OK(test, CFCType_is_composite(type), "composite type %s",
               type_string);
            CFCBase_decref((CFCBase*)type);
        }
    }

    {
        CFCType *foo = CFCType_new_object(0, NULL, "Foo", 1);
        CFCType *const_foo = CFCType_new_object(CFCTYPE_CONST, NULL, "Foo", 1);

        CFCType *composite = CFCType_new_composite(0, foo, 1, NULL);
        OK(test, CFCType_is_composite(composite), "is_composite");
        STR_EQ(test, CFCType_get_specifier(composite), "Foo",
               "get_specifier delegates to child" );

        CFCType *twin = CFCType_new_composite(0, foo, 1, NULL);
        OK(test, CFCType_equals(composite, twin), "equals");
        CFCBase_decref((CFCBase*)twin);

        CFCType *const_composite
            = CFCType_new_composite(0, const_foo, 1, NULL);
        OK(test, !CFCType_equals(composite, const_composite),
           "equals spoiled by different child");
        CFCBase_decref((CFCBase*)const_composite);

        CFCBase_decref((CFCBase*)composite);
        CFCBase_decref((CFCBase*)foo);
        CFCBase_decref((CFCBase*)const_foo);
    }

    {
        CFCClass *class_list[1] = { NULL };
        CFCType *foo_array = CFCTest_parse_type(test, parser, "foo_t[]");
        CFCType_resolve(foo_array, class_list);
        STR_EQ(test, CFCType_get_array(foo_array), "[]", "get_array");
        STR_EQ(test, CFCType_to_c(foo_array), "foo_t",
               "array subscripts not included by to_c");
        CFCType *foo_array_array
            = CFCTest_parse_type(test, parser, "foo_t[][]");
        OK(test, !CFCType_equals(foo_array, foo_array_array),
           "equals spoiled by different array postfixes");

        CFCBase_decref((CFCBase*)foo_array);
        CFCBase_decref((CFCBase*)foo_array_array);
    }

    {
        CFCType *foo_star = CFCTest_parse_type(test, parser, "foo_t*");
        CFCType *foo_star_star = CFCTest_parse_type(test, parser, "foo_t**");
        OK(test, !CFCType_equals(foo_star, foo_star_star),
           "equals spoiled by different levels of indirection");
        INT_EQ(test, CFCType_get_indirection(foo_star), 1,
               "foo_t* indirection");
        INT_EQ(test, CFCType_get_indirection(foo_star_star), 2,
               "foo_t** indirection");

        CFCBase_decref((CFCBase*)foo_star);
        CFCBase_decref((CFCBase*)foo_star_star);
    }

    CFCBase_decref((CFCBase*)parser);
}

