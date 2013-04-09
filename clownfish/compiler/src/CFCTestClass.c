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

#include <string.h>

#define CFC_USE_TEST_MACROS
#include "CFCBase.h"
#include "CFCClass.h"
#include "CFCFileSpec.h"
#include "CFCFunction.h"
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

static int
S_has_symbol(CFCSymbol **symbols, const char *micro_sym);

const CFCTestBatch CFCTEST_BATCH_CLASS = {
    "Clownfish::CFC::Model::Class",
    83,
    S_run_tests
};

static void
S_run_tests(CFCTest *test) {
    CFCParser *parser = CFCParser_new();

    CFCParcel *neato = CFCParcel_new("Neato", NULL, NULL);
    CFCFileSpec *file_spec = CFCFileSpec_new(".", "Foo/FooJr", 0);

    CFCVariable *thing;
    CFCVariable *widget;
    CFCFunction *tread_water;

    {
        CFCType *thing_type = CFCTest_parse_type(test, parser, "Thing*");
        thing = CFCVariable_new(neato, NULL, "Foo", NULL, "thing",
                                thing_type, 0);

        CFCType *widget_type = CFCTest_parse_type(test, parser, "Widget*");
        widget = CFCVariable_new(NULL, NULL, "Widget", NULL, "widget",
                                 widget_type, 0);

        CFCType *return_type = CFCTest_parse_type(test, parser, "void");
        CFCParamList *param_list
            = CFCTest_parse_param_list(test, parser, "()");
        tread_water
            = CFCFunction_new(neato, NULL, "Foo", NULL, "tread_water",
                              return_type, param_list, NULL, 0);

        CFCBase_decref((CFCBase*)thing_type);
        CFCBase_decref((CFCBase*)widget_type);
        CFCBase_decref((CFCBase*)return_type);
        CFCBase_decref((CFCBase*)param_list);
    }

    CFCClass *foo
        = CFCClass_create(neato, NULL, "Foo", NULL, NULL, NULL, NULL, NULL,
                          0, 0);
    CFCClass_add_function(foo, tread_water);
    CFCClass_add_member_var(foo, thing);
    CFCClass_add_inert_var(foo, widget);

    {
        CFCClass *should_be_foo = CFCClass_fetch_singleton(neato, "Foo");
        OK(test, should_be_foo == foo, "fetch_singleton");
    }

    CFCClass *foo_jr
        = CFCClass_create(neato, NULL, "Foo::FooJr", NULL, NULL, NULL, NULL,
                          "Foo", 0, 0);
    CFCClass_add_attribute(foo_jr, "dumpable", "1");
    OK(test, CFCClass_has_attribute(foo_jr, "dumpable"), "has_attribute");
    STR_EQ(test, CFCClass_get_struct_sym(foo_jr), "FooJr",
           "get_struct_sym");
    STR_EQ(test, CFCClass_full_struct_sym(foo_jr), "neato_FooJr",
           "full_struct_sym");

    CFCClass *final_foo
        = CFCClass_create(neato, NULL, "Foo::FooJr::FinalFoo", NULL, NULL, NULL,
                          file_spec, "Foo::FooJr", 1, 0);
    CFCClass_add_attribute(final_foo, "dumpable", "1");
    OK(test, CFCClass_final(final_foo), "final");
    STR_EQ(test, CFCClass_include_h(final_foo), "Foo/FooJr.h",
           "include_h uses path_part");
    STR_EQ(test, CFCClass_get_parent_class_name(final_foo), "Foo::FooJr",
           "get_parent_class_name");

    {
        CFCParcel *parsed_neato
            = CFCTest_parse_parcel(test, parser, "parcel Neato;");
        CFCBase_decref((CFCBase*)parsed_neato);
    }

    CFCParser_set_class_name(parser, "Foo");
    CFCMethod *do_stuff
        = CFCTest_parse_method(test, parser, "void Do_Stuff(Foo *self);");
    CFCClass_add_method(foo, do_stuff);

    CFCClass_add_child(foo, foo_jr);
    CFCClass_add_child(foo_jr, final_foo);
    CFCClass_grow_tree(foo);

    OK(test, CFCClass_get_parent(foo_jr) == foo, "grow_tree, one level" );
    OK(test, CFCClass_get_parent(final_foo) == foo_jr,
       "grow_tree, two levels");
    OK(test, CFCClass_fresh_method(foo, "Do_Stuff") == do_stuff,
       "fresh_method");
    OK(test, CFCClass_method(foo_jr, "Do_Stuff") == do_stuff,
       "inherited method");
    OK(test, CFCClass_fresh_method(foo_jr, "Do_Stuff") == NULL,
       "inherited method not 'fresh'");
    OK(test, CFCMethod_final(CFCClass_method(final_foo, "Do_Stuff")),
       "Finalize inherited method");
    OK(test, !CFCMethod_final(CFCClass_method(foo_jr, "Do_Stuff")),
       "Don't finalize method in parent");

    {
        CFCVariable **inert_vars = CFCClass_inert_vars(foo);
        OK(test, inert_vars[0] == widget, "inert_vars[0]");
        OK(test, inert_vars[1] == NULL, "inert_vars[1]");

        CFCFunction **functions = CFCClass_functions(foo);
        OK(test, functions[0] == tread_water, "functions[0]");
        OK(test, functions[1] == NULL, "functions[1]");

        CFCMethod **methods = CFCClass_methods(foo);
        OK(test, methods[0] == do_stuff, "methods[0]");
        OK(test, methods[1] == NULL, "methods[1]");

        CFCMethod **fresh_methods = CFCClass_fresh_methods(foo);
        OK(test, fresh_methods[0] == do_stuff, "fresh_methods[0]");
        OK(test, fresh_methods[1] == NULL, "fresh_methods[1]");

        CFCVariable **fresh_member_vars = CFCClass_fresh_member_vars(foo);
        OK(test, fresh_member_vars[0] == thing, "fresh_member_vars[0]");
        OK(test, fresh_member_vars[1] == NULL, "fresh_member_vars[1]");

        FREEMEM(fresh_methods);
        FREEMEM(fresh_member_vars);
    }

    {
        CFCVariable **member_vars = CFCClass_member_vars(foo_jr);
        OK(test, member_vars[0] == thing, "member_vars[0]");
        OK(test, member_vars[1] == NULL, "member_vars[1]");

        CFCFunction **functions = CFCClass_functions(foo_jr);
        OK(test, functions[0] == NULL, "functions[0]");

        CFCVariable **fresh_member_vars = CFCClass_fresh_member_vars(foo_jr);
        OK(test, fresh_member_vars[0] == NULL, "fresh_member_vars[0]");

        CFCVariable **inert_vars = CFCClass_inert_vars(foo_jr);
        OK(test, inert_vars[0] == NULL, "inert_vars[0]");

        FREEMEM(fresh_member_vars);
    }

    {
        CFCMethod **fresh_methods = CFCClass_fresh_methods(final_foo);
        OK(test, fresh_methods[0] == NULL, "fresh_methods[0]");
        FREEMEM(fresh_methods);
    }

    {
        const char *autocode = CFCClass_get_autocode(foo_jr);
        OK(test, strstr(autocode, "load") != NULL, "autogenerate Dump/Load");
    }

    {
        CFCClass **ladder = CFCClass_tree_to_ladder(foo);
        OK(test, ladder[0] == foo, "ladder[0]");
        OK(test, ladder[1] == foo_jr, "ladder[1]");
        OK(test, ladder[2] == final_foo, "ladder[2]");
        OK(test, ladder[3] == NULL, "ladder[3]");
        FREEMEM(ladder);
    }

    {
        CFCClass *final_class
            = CFCTest_parse_class(test, parser, "final class Iamfinal { }");
        OK(test, CFCClass_final(final_class), "class modifer: final");
        CFCClass *inert_class
            = CFCTest_parse_class(test, parser, "inert class Iaminert { }");
        OK(test, CFCClass_inert(inert_class), "class modifer: inert");

        CFCBase_decref((CFCBase*)final_class);
        CFCBase_decref((CFCBase*)inert_class);
    }

    {
        static const char *names[2] = { "Fooble", "Foo::FooJr::FooIII" };
        for (int i = 0; i < 2; ++i) {
            const char *name = names[i];
            char *class_src
                = CFCUtil_sprintf("class Fu::%s inherits %s { }", name, name);
            CFCClass *klass = CFCTest_parse_class(test, parser, class_src);
            STR_EQ(test, CFCClass_get_parent_class_name(klass), name,
                   "class_inheritance: %s", name);
            FREEMEM(class_src);
            CFCBase_decref((CFCBase*)klass);
        }
    }

    {
        const char *class_src =
            "public class Foo::Foodie cnick Foodie inherits Foo {\n"
            "    int num;\n"
            "}\n";
        CFCClass *klass = CFCTest_parse_class(test, parser, class_src);
        CFCSymbol **member_vars = (CFCSymbol**)CFCClass_member_vars(klass);
        OK(test, S_has_symbol(member_vars, "num"),
           "parsed member var");

        CFCBase_decref((CFCBase*)klass);
    }

    {
        const char *class_src =
            "/**\n"
            " * Bow wow.\n"
            " *\n"
            " * Wow wow wow.\n"
            " */\n"
            "public class Animal::Dog inherits Animal : lovable : drooly {\n"
            "    public inert Dog* init(Dog *self, CharBuf *name,\n"
            "                           CharBuf *fave_food);\n"
            "    inert uint32_t count();\n"
            "    inert uint64_t num_dogs;\n"
            "    public inert Dog *top_dog;\n"
            "\n"
            "    CharBuf *name;\n"
            "    bool     likes_to_go_fetch;\n"
            "    ChewToy *squishy;\n"
            "\n"
            "    void               Destroy(Dog *self);\n"
            "    public CharBuf*    Bark(Dog *self);\n"
            "    public void        Eat(Dog *self);\n"
            "    public void        Bite(Dog *self, Enemy *enemy);\n"
            "    public Thing      *Fetch(Dog *self, Thing *thing);\n"
            "    public final void  Bury(Dog *self, Bone *bone);\n"
            "    public Owner      *mom;\n"
            "    public abstract incremented nullable Thing*\n"
            "    Scratch(Dog *self);\n"
            "\n"
            "    int32_t[1]  flexible_array_at_end_of_struct;\n"
            "}\n";
        CFCClass *klass = CFCTest_parse_class(test, parser, class_src);

        CFCSymbol **inert_vars  = (CFCSymbol**)CFCClass_inert_vars(klass);
        CFCSymbol **member_vars = (CFCSymbol**)CFCClass_member_vars(klass);
        CFCSymbol **functions   = (CFCSymbol**)CFCClass_functions(klass);
        CFCSymbol **methods     = (CFCSymbol**)CFCClass_methods(klass);
        OK(test, S_has_symbol(inert_vars, "num_dogs"), "parsed inert var");
        OK(test, S_has_symbol(inert_vars, "top_dog"), "parsed public inert var");
        OK(test, S_has_symbol(member_vars, "mom"), "parsed public member var");
        OK(test, S_has_symbol(member_vars, "squishy"),
           "parsed parcel member var");
        OK(test, S_has_symbol(functions, "init"), "parsed function");
        OK(test, S_has_symbol(methods, "destroy"), "parsed parcel method");
        OK(test, S_has_symbol(methods, "bury"), "parsed public method");
        OK(test, S_has_symbol(methods, "scratch"),
           "parsed public abstract nullable method");

        CFCMethod *scratch = CFCClass_method(klass, "scratch");
        OK(test, scratch != NULL, "find method 'scratch'");
        OK(test, CFCType_nullable(CFCMethod_get_return_type(scratch)),
           "public abstract incremented nullable flagged as nullable");

        int num_public_methods = 0;
        for (int i = 0; methods[i]; ++i) {
            if (CFCSymbol_public(methods[i])) { ++num_public_methods; }
        }
        INT_EQ(test, num_public_methods, 6, "pass acl to Method constructor");

        OK(test, CFCClass_has_attribute(klass, "lovable"),
           "parsed class attribute");
        OK(test, CFCClass_has_attribute(klass, "drooly"),
           "parsed second class attribute");

        CFCBase_decref((CFCBase*)klass);
    }

    {
        const char *class_src =
            "inert class Rigor::Mortis cnick Mort {\n"
            "    inert void lie_still();\n"
            "}\n";
        CFCClass *klass = CFCTest_parse_class(test, parser, class_src);
        OK(test, CFCClass_inert(klass),
           "inert modifier parsed and passed to constructor");

        CFCBase_decref((CFCBase*)klass);
    }

    {
        const char *class_src =
            "final class Ultimo {\n"
            "    /** Throws an error.\n"
            "     */\n"
            "    void Say_Never(Ultimo *self);\n"
            "}\n";
        CFCClass *klass = CFCTest_parse_class(test, parser, class_src);
        OK(test, CFCClass_final(klass), "final class_declaration");
        CFCBase_decref((CFCBase*)klass);
    }

    CFCBase_decref((CFCBase*)parser);
    CFCBase_decref((CFCBase*)neato);
    CFCBase_decref((CFCBase*)file_spec);
    CFCBase_decref((CFCBase*)thing);
    CFCBase_decref((CFCBase*)widget);
    CFCBase_decref((CFCBase*)tread_water);
    CFCBase_decref((CFCBase*)foo);
    CFCBase_decref((CFCBase*)foo_jr);
    CFCBase_decref((CFCBase*)final_foo);
    CFCBase_decref((CFCBase*)do_stuff);

    CFCClass_clear_registry();
    CFCParcel_reap_singletons();
}

static int
S_has_symbol(CFCSymbol **symbols, const char *micro_sym) {
    for (int i = 0; symbols[i]; ++i) {
        if (strcmp(CFCSymbol_micro_sym(symbols[i]), micro_sym) == 0) {
            return 1;
        }
    }

    return 0;
}

